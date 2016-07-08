# The MIT License (MIT)
#
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
from minilocker import Point3d
from collections import deque

class BaseFilter:

    def push(self, pt):
        return pt


class CompositeFilter(BaseFilter):

    def __init__(self, *filts):
        self.filts = filts

    def push(self, pt):
        for f in self.filts:
            pt = f.push(pt)
            if pt is None:
                return None
        return pt


class ScaleFilter(BaseFilter):

    def __init__(self, scale):
        self.scale = scale

    def push(self, pt):
        return pt * self.scale


class DecimatingFIRFilter(BaseFilter):

    def __init__(self, coeffs, decimate = 1, normalize = True, wait_for_fill = True):
        if normalize:
            sm = sum(coeffs)
            self.coeffs = [x / sm for x in coeffs]
        else:
            self.coeffs = coeffs
        self.decimate = decimate
        self._que = deque([Point3d.ZERO] * len(self.coeffs), maxlen = len(self.coeffs))
        self._filled = len(self.coeffs) if wait_for_fill else 0
        self._at = self.decimate - 1

    def push(self, pt):
        self._que.append(pt)
        if self._filled > 0:
            self._filled -= 1
            return None
        if self._at != self.decimate - 1:
            self._at += 1
            return None
        self._at = 0
        res = Point3d.ZERO
        for v, c in zip(self._que, reversed(self.coeffs)):
            res += v * c
        return res
