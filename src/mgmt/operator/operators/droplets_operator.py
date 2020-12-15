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
import random
from common.constants import *
from common.common import *
from common.object_operator import ObjectOperator
from kubernetes import client, config
from obj.droplet import Droplet
from store.operator_store import OprStore

logger = logging.getLogger()


class DropletOperator(ObjectOperator):
    _instance = None

    def __new__(cls, **kwargs):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls.__init__(cls, **kwargs)
        return cls._instance

    def __init__(self, **kwargs):
        logger.info(kwargs)
        self.store = OprStore()
        config.load_incluster_config()
        self.obj_api = client.CustomObjectsApi()
        self.allocated_droplets = set()

    def query_existing_droplets(self):
        def list_droplet_obj_fn(name, spec, plurals):
            logger.info("Bootstrapped droplet {}".format(name))
            droplet = Droplet(name, self.obj_api, self.store, spec)
            if droplet.status == OBJ_STATUS.obj_status_provisioned:
                self.store.update_obj(droplet)

        kube_list_obj(self.obj_api, RESOURCES.droplets, list_droplet_obj_fn)
        self.bootstrapped = True

    def get_stored_obj(self, name, spec):
        return Droplet(name, self.obj_api, self.store, spec)

    def get_droplet_by_ip(self, ip):
        for d in self.store.store["Droplet"]:
            if self.store.store["Droplet"][d].ip == ip:
                return self.store.store["Droplet"][d]
        return None

    def assign_droplet(self, obj, network):
        droplets = set(self.store.get_all_network_droplets(
            network)) - self.allocated_droplets
        if len(droplets) == 0:
            return False
        d = random.sample(droplets, 1)[0]
        obj.droplet = d
        self.allocated_droplets.add(d)
        logger.info("Assigned droplet {} to {}".format(d, obj.name))
        return True

    def unassign_droplet(self, obj):
        if obj.droplet == "":
            return False
        self.allocated_droplets.remove(obj.droplet)
        logger.info("Unassigned droplet {} from {}".format(
            obj.droplet, obj.name))
        obj.droplet = ""
        return True

    def get_unallocated_droplets(self):
        return set(self.store.get_all_obj_type(
            KIND.droplet)) - self.allocated_droplets
