############################################################################
# tools/gdb/nuttxgdb/kasan.py
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

import traceback

import gdb

from . import utils


def get_struct_ptr(addr, struct):
    struct_type = gdb.lookup_type(struct)
    return gdb.Value(addr).cast(struct_type.pointer())


def parse_args(args):
    return [int(arg) for arg in args.split()]


class KASanRegion:
    def __init__(self, begin, end, shadow, bitwidth, scale):
        self.begin = begin
        self.end = end
        self.shadow = shadow
        self.bitwidth = bitwidth
        self.scale = scale

    def check_addr(self, addr):
        distance = addr - self.begin
        distance = distance / self.scale
        index = distance / self.bitwidth
        bit = distance % self.bitwidth

        return self.shadow[index] >> bit & 0x01


class KASan(gdb.Command):

    def __init__(self):
        super().__init__("kasan-debug", gdb.COMMAND_USER)

        bitwidth = utils.get_symbol_value("sizeof(unsigned long)")
        scale = utils.get_symbol_value("KASAN_SHADOW_SCALE")
        regions = utils.get_symbol_value("g_region")
        self.regions = []

        for addr in regions:
            region = get_struct_ptr(addr, "struct kasan_region_s")
            print("begin: 0x%x" % (region["begin"]), end=" ")
            print("end: 0x%x" % (region["end"]), end=" ")
            print("scale 0x%x" % (scale))
            self.regions.append(
                KASanRegion(region["begin"], region["end"], region["shadow"], scale)
            )

    def invoke(self, args, from_tty):
        addrs = parse_args(args)
        for addr in addrs:
            for region in self.regions:
                if region.begin <= addr <= region.end:
                    if region.check_addr(addr):
                        print("Addr 0x%x Error" % (addr))
                    else:
                        print("Addr 0x%x OK" % (addr))
