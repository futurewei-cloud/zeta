# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import json
from common.constants import *
from common.common import *
from common.kube_obj import KubeObject


class Chain(KubeObject):

    def __init__(self, name, obj_api, opr_store, spec=None):
        super().__init__(name, obj_api, opr_store, spec)
        self.kind = "Chain"
        self.plural = "chains"
        self.head = ""
        self.tail = ""
        self.dft = ""
        self.size = 0
        self.ftns = []
        if spec is not None:
            self.set_obj_spec(spec)

    def get_obj_spec(self):
        self.obj = {
            "id": self.id,
            "status": self.status,
            "head": self.head,
            "tail": self.tail,
            "ftns": self.ftns,
            "dft": self.dft,
            "size": self.size
        }
        return self.obj

    def set_obj_spec(self, spec):
        # K8s APIs
        self.id = get_spec_val('id', spec)
        self.status = get_spec_val('status', spec)
        self.head = get_spec_val('head', spec)
        self.tail = get_spec_val('tail', spec)
        self.ftns = get_spec_val('ftns', spec)
        self.dft = get_spec_val('dft', spec)
        self.size = get_spec_val('size', spec)
