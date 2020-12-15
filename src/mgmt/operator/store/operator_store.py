# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:The above copyright
# notice and this permission notice shall be included in all copies or
# substantial portions of the Software.THE SOFTWARE IS PROVIDED "AS IS",
# WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import logging
import inspect
from common.constants import KIND
from common.constants import OBJ_DEFAULTS

logger = logging.getLogger()


class OprStore(object):
    _instance = None

    def __new__(cls, **kwargs):
        if cls._instance is None:
            cls._instance = super(OprStore, cls).__new__(cls)
            cls._init(cls, **kwargs)
        return cls._instance

    def _init(self, **kwargs):
        logger.info(kwargs)
        self.store = {}
        self.store[KIND.fwd] = {}
        self.store[KIND.ftn] = {}
        self.store[KIND.chain] = {}
        self.store[KIND.dft] = {}

        self.store[KIND.droplet] = {}
        self.store[OBJ_DEFAULTS.zgc_net] = {}
        self.store[OBJ_DEFAULTS.tenant_net] = {}

    def update_obj(self, obj):
        if obj.kind in self.store:
            if obj.kind == KIND.droplet:
                # Special case to differentiate between ZGC and Tenant droplets
                self.store[obj.network][obj.name] = obj
            self.store[obj.kind][obj.name] = obj
        else:
            logger.info("Unknown object type {}".format(obj.kind))

    def get_obj(self, name, kind):
        if kind in self.store:
            if name in self.store[kind]:
                return self.store[kind][name]
            logger.info(
                "Object {} of kind {} not found".format(name, kind))
            return None
        else:
            logger.info("Unknown object type {}".format(kind))
            return None

    def delete_obj(self, name, kind):
        if kind in self.store:
            if name in self.store[kind]:
                del self.store[kind][name]
            logger.info(
                "Object {} of kind {} not found".format(kind, name))
        else:
            logger.info("Unknown object type {}".format(kind))

    def get_all_obj_type(self, kind):
        return self.store[kind].keys()

    def contains_obj(self, name, kind):
        if name in self.store[kind]:
            return True
        return False

    def dump_obj_kind(self, kind):
        for obj in self.store[kind].values():
            logger.info("{}: {}, Spec: {}".format(
                kind, obj.name, obj.get_obj_spec()))

    def get_all_network_droplets(self, network):
        return self.store[network].keys()
