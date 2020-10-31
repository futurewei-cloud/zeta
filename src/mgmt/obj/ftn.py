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
        super().__init__(name, obj_api, opr_store, spec)
        self.kind = "Ftn"
        self.plural = "ftns"
        self.ftn = ""
        self.ip = ""
        self.mac = ""
        self.nextnode = ""
        self.droplet = ""
        self.parent_chain = ""
        self.augmented_chain = ""
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
            "parentchain": self.parent_chain,
            "augmentedchain": self.augmented_chain
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
        self.parent_chain = get_spec_val('parentchain', spec)
        self.augmented_chain = get_spec_val('augmentedchain', spec)
