# The MIT License (MIT)
#
# Copyright (C) 2015-1016 Institute of Applied and Experimental Physics (http://www.utef.cvut.cz/)
# Copyright (C) 2016 Josef Gajdusek <atx@atx.name>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import asyncio
import serial.aio
import functools
import operator
import struct
import collections

class Point3d(collections.namedtuple("Point3d", ["x", "y", "z"])):

    def __add__(self, other):
        return Point3d(x = self.x + other.x,
                       y = self.y + other.y,
                       z = self.z + other.z)

    def __sub__(self, other):
        return Point3d(x = self.x - other.x,
                       y = self.y - other.y,
                       z = self.z - other.z)

    def __mul__(self, other):
        return Point3d(x = self.x * other,
                       y = self.y * other,
                       z = self.z * other)
    __rmul__ = __mul__

    def __truediv__(self, other):
        return Point3d(x = self.x / other,
                       y = self.y / other,
                       z = self.z / other)
    __rtruediv__ = __truediv__

Point3d.ZERO = Point3d(0, 0, 0)


# Originally from https://github.com/atalax/spectrometer/blob/master/interface/standalone/spectrometer.py

def unpack_3d(b):
    x, y, z = struct.unpack("<3h", b)
    return Point3d(x = x, y = y, z = z)

class MiniLocker:

    # Host->Device
    PACK_NOP = 0x01
    PACK_PING = 0x02
    PACK_GET = 0x03
    PACK_SET = 0x04
    PACK_START = 0x05
    PACK_STOP = 0x06

    # Device->Host
    PACK_PONG = 0x82
    PACK_GETRESP = 0x83
    PACK_IMU = 0x84
    PACK_MAGNET = 0x85
    PACK_ERROR = 0xff

    PROP_FW = 0x01
    PROP_SERNO = 0x02
    PROP_MAGCALX = 0x03
    PROP_MAGCALY = 0x04
    PROP_MAGCALZ = 0x05

    PACK_LENGTH_MAP = {
        PACK_PONG: 1,
        PACK_GETRESP: None, # See PROP_LENGTH_MAP
        PACK_IMU: 13,
        PACK_MAGNET: 7,
        PACK_ERROR: 2,
    }

    PROP_LENGTH_MAP = {
        PROP_FW: 2,
        PROP_SERNO: 2,
        PROP_MAGCALX: 1,
        PROP_MAGCALY: 1,
        PROP_MAGCALZ: 1,
    }

    _initbaud = 115200 # Does not matter really

    def __init__(self):
        self._initsem = asyncio.Semaphore(value = 0)
        self._recvqueue = asyncio.Queue()
        self.event_loop = asyncio.get_event_loop()
        self._packqueues = [asyncio.Queue() for _ in range(256)]
        self._packlock = asyncio.Lock()
        self._proplock = asyncio.Lock()
        self.magbias = Point3d.ZERO

        self._transport = None

    def connection_made(self, transport):
        self._transport = transport
        self._initsem.release()

    def data_received(self, data):
        for x in range(len(data)):
            self._recvqueue.put_nowait(data[x:x + 1])

    async def recv(self, nbytes = 1):
        ret = b""
        for _ in range(nbytes):
            ret += await self._recvqueue.get()
        return ret

    @classmethod
    async def connect(cls, port):
        transport, spect = await serial.aio.create_serial_connection(
                                asyncio.get_event_loop(), cls, port, cls._initbaud)
        await spect._initsem.acquire()
        await spect._ainit()
        return spect

    async def _ainit(self):
        self._transport.write([MiniLocker.PACK_NOP] * 100) # Flush the device buffer if it has not been flushed yet
        self.flush()
        asyncio.ensure_future(self._recv_loop())

        ver = await self.get_prop(MiniLocker.PROP_FW)
        self.fw_version = "%d.%d" % (ver >> 8, ver & 0xff)

        self.serno = await self.get_prop(MiniLocker.PROP_SERNO)

        self._magcal = []
        # See AK89653 datasheet 8.3.11
        for p in [MiniLocker.PROP_MAGCALX, MiniLocker.PROP_MAGCALY, MiniLocker.PROP_MAGCALZ]:
            raw = await self.get_prop(p)
            self._magcal.append(((raw - 128) * 0.5) / 128 + 1)
        self._magmax = [32767 * c for c in self._magcal]

        self.start()

    def start(self):
        self.send_packet(MiniLocker.PACK_START)

    def stop(self):
        self.send_packet(MiniLocker.PACK_STOP)

    async def _recv_loop(self):
        while True:
            pack = await self.recv_packet()
            self._packqueues[pack[0]].put_nowait(pack)

    @staticmethod
    def _encode_lendian(val, ln):
        return functools.reduce(operator.add, [bytes([(val >> (i * 8)) & 0xff]) for i in range(ln)])

    @staticmethod
    def _decode_lendian(bytss):
        return functools.reduce(operator.add,
                                map(lambda i, v: v << (i * 8), *zip(*enumerate(bytss))))

    async def ping(self):
        self.send_packet(MiniLocker.PACK_PING)
        await self.recv_packet_queued(MiniLocker.PACK_PONG)

    def set_prop(self, prop, val):
        self.send_packet(MiniLocker.PACK_SET,
                         prop,
                         MiniLocker._encode_lendian(val,
                                                    MiniLocker.PROP_LENGTH_MAP[prop]))

    async def get_prop(self, prop):
        async with self._proplock:
            self.send_packet(MiniLocker.PACK_GET, prop)
            pack = await self.recv_packet_queued(MiniLocker.PACK_GETRESP)
            return MiniLocker._decode_lendian(pack[2:])

    async def recv_packet_queued(self, typ):
        return await self._packqueues[typ].get()

    def send_packet(self, *args):
        self._transport.write(
            functools.reduce(
                    operator.add,
                    map(bytes, [[x] if isinstance(x, int) else x for x in args])))

    async def recv_packet(self):
        async with self._packlock:
            while True:
                pack = await self.recv(1)
                typ = pack[0]
                if typ == MiniLocker.PACK_GETRESP:
                    pack += await self.recv(1)
                    propid = pack[-1]
                    return pack + (await self.recv(MiniLocker.PROP_LENGTH_MAP[propid]))
                elif typ in MiniLocker.PACK_LENGTH_MAP:
                    ln = MiniLocker.PACK_LENGTH_MAP[typ]
                    return pack + ((await self.recv(ln - 1)) if ln > 1 else b"")
                # Drop the byte

    async def next_imu(self):
        p = await self.recv_packet_queued(MiniLocker.PACK_IMU)
        return (unpack_3d(p[1:7]) / 32767, unpack_3d(p[7:13]) / 32767)

    async def next_magnet(self):
        p = await self.recv_packet_queued(MiniLocker.PACK_MAGNET)
        return Point3d(*[(v * c) / x for v, c, x in
                         zip(list(unpack_3d(p[1:])), self._magcal, self._magmax)]) - self.magbias

    def flush(self):
        self._recvqueue = asyncio.Queue()

    def close(self):
        self.stop()
        self._transport.close()
