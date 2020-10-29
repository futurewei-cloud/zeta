# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import json
from common.constants import *
from common.common import *
from common.kube_obj import KubeObject
logger = logging.getLogger()


class Ftn(KubeObject):

    def __init__(self, name, obj_api, opr_store, spec=None):
        super.__init__(name, obj_api, opr_store, spec)
        self.ftn = ""
        self.ip = ""
        self.mac = ""
        self.nextnode = ""
        self.droplet = ""
        self.chains = []
        if spec is not None:
            self.set_obj_spec(spec)

    def get_obj_spec(self):
        self.obj = {
            "status": self.status,
            "ftn": self.ftn,
            "ip": self.ip,
            "mac": self.mac,
            "nextnode": self.nextnode,
            "droplet": self.droplet,
            "chain": self.chains
        }

        return self.obj

    def set_obj_spec(self, spec):
        # K8s APIs
        self.status = get_spec_val('status', spec)
        self.ftn = get_spec_val('ftn', spec)
        self.ip = get_spec_val('ip', spec)
        self.mac = get_spec_val('mac', spec)
        self.nextnode = get_spec_val('nextnode', spec)
        self.droplet = get_spec_val('droplet', spec)
        self.chains = get_spec_val('chains', spec)

    def get_plural(self):
        return "ftns"

    def get_kind(self):
        return "Ftn"

    def store_update_obj(self):
        if self.store is None:
            return
        self.store.update_ftn(self)

    def store_delete_obj(self):
        if self.store is None:
            return
        self.store.delete_ftn(self.name)
