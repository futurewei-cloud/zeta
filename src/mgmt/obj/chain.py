# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import json
from common.constants import *
from common.common import *
from common.kube_obj import KubeObject
logger = logging.getLogger()


class Chain(KubeObject):

    def __init__(self, name, obj_api, opr_store, spec=None):
        super.__init__(name, obj_api, opr_store, spec)
        self.head = ""
        self.tail = ""
        self.ftns = []
        if spec is not None:
            self.set_obj_spec(spec)

    def get_obj_spec(self):
        self.obj = {
            "status": self.status,
            "head": self.head,
            "tail": self.tail,
            "ftns": self.ftns
        }
        return self.obj

    def set_obj_spec(self, spec):
        # K8s APIs
        self.status = get_spec_val('status', spec)
        self.head = get_spec_val('head', spec)
        self.tail = get_spec_val('tail', spec)
        self.ftns = get_spec_val('ftns', spec)

    def get_plural(self):
        return "chains"

    def get_kind(self):
        return "Chain"

    def store_update_obj(self):
        if self.store is None:
            return
        self.store.update_chain(self)

    def store_delete_obj(self):
        if self.store is None:
            return
        self.store.delete_chain(self.name)
