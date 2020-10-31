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
        self.store = {}
        self.store["droplets"] = {}
        self.store["ftns"] = {}
        self.store["chains"] = {}
        self.store["dfts"] = {}

    def update_obj(self, obj):
        if obj.resource in self.store:
            self.store[obj.resource][obj.name] = obj
        else:
            logger.info("Unknown object type {}".format(obj.resource))

    def get_obj(self, name, resource):
        if resource in self.store:
            if name in self.store[resource]:
                return self.store[resource][name]
            logger.info(
                "Object {} of resource {} not found".format(resource, name))
            return None
        else:
            logger.info("Unknown object type {}".format(resource))
            return None

    def delete_obj(self, name, resource):
        if resource in self.store:
            if name in self.store[resource]:
                del self.store[resource][name]
            logger.info(
                "Object {} of resource {} not found".format(resource, name))
        else:
            logger.info("Unknown object type {}".format(resource))

    def get_droplet_by_ip(self, ip):
        for d in self.store["Droplet"]:
            if self.store["Droplet"][d].ip == ip:
                return self.store["Droplet"][d]
        return None

    def get_all_droplets(self):
        return self.store["Droplet"].values()

    def contains_droplet(self, name):
        if name in self.store["Droplet"]:
            return True
        return False

    def dump_droplets(self):
        for d in self.store["Droplet"].values():
            logger.info("Droplets: {}, Spec: {}".format(
                d.name, d.get_obj_spec()))
