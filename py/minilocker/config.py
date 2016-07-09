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

import glob
import minilocker
import os
import yaml

CONFIG_PATH = "~/.minilocker"

DEFAULT_CONFIG = yaml.load("""
port: /dev/ttyACM*
cal:
  magnetometer:
    bias: [0.0, 0.0, 0.0]
    scale: [1.0, 1.0, 1.0]
""")

def merge_configs(*args):
    ret = {}
    for m in args:
        for k, v in m.items():
            if isinstance(v, map):
                ret[k] = merge_configs(*[x for x in args if k in x])
            else:
                ret[k] = v
    return ret

def load_config(path = CONFIG_PATH):
    path = os.path.expanduser(path)
    ret = DEFAULT_CONFIG
    if os.path.exists(path):
        with open(path, "r") as f:
            ret = merge_configs(ret, yaml.load(f))
    return ret

def save_config(cfg, path = CONFIG_PATH):
    path = os.path.expanduser(path)
    with open(path, "w+") as f:
        yaml.dump(cfg, stream = f)


def cli_add_default_args(parser):
    parser.add_argument(
        "-s", "--serial",
        help = "Serial port to use",
        default = None
    )
    parser.add_argument(
        "-c", "--config",
        help = "Config file to use",
        default = CONFIG_PATH
    )

async def cli_connect(args, load_cal = True):
    cfg = load_config(args.config)
    ml = await minilocker.MiniLocker.connect(
                    glob.glob(args.serial if args.serial else cfg["port"])[0])
    if load_cal:
        ml.set_magcal(cfg["cal"]["magnetometer"]["scale"])
        ml.magbias = minilocker.Point3d(*cfg["cal"]["magnetometer"]["bias"])
    return cfg, ml
