# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import json
from common.constants import *
from common.common import *
from common.kube_obj import KubeObject
logger = logging.getLogger()


class Fwd(KubeObject):

    def __init__(self, name, obj_api, opr_store, spec=None):
        super().__init__(name, obj_api, opr_store, spec)
        self.kind = "Fwd"
        self.plural = "fwds"
        self.dft = ""
        self.droplet = ""
        if spec is not None:
            self.set_obj_spec(spec)

    def get_obj_spec(self):
        self.obj = {
            "id": self.id,
            "status": self.status,
            "dft": self.dft,
            "droplet": self.droplet
        }

        return self.obj

    def set_obj_spec(self, spec):
        # K8s APIs
        self.id = get_spec_val('id', spec)
        self.status = get_spec_val('status', spec)
        self.dft = get_spec_val('dft', spec)
        self.droplet = get_spec_val('droplet', spec)
