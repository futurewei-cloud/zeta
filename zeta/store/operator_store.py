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
        self.droplets_store = {}

    def update_droplet(self, droplet):
        self.droplets_store[droplet.name] = droplet

    def delete_droplet(self, name):
        if name in self.droplets_store:
            del self.droplets_store[name]

    def get_droplet(self, name):
        if name in self.droplets_store:
            return self.droplets_store[name]
        return None

    def get_droplet_by_ip(self, ip):
        for d in self.droplets_store:
            if self.droplets_store[d].ip == ip:
                return self.droplets_store[d]
        return None

    def get_all_droplets(self):
        return self.droplets_store.values()

    def contains_droplet(self, name):
        if name in self.droplets_store:
            return True
        return False

    def _dump_droplets(self):
        for d in self.droplets_store.values():
            logger.info("Droplets: {}, Spec: {}".format(
                d.name, d.get_obj_spec()))
