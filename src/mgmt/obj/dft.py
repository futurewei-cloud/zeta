# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import json
from common.constants import *
from common.common import *
from common.kube_obj import KubeObject
logger = logging.getLogger()


class Dft(KubeObject):

    def __init__(self, name, obj_api, opr_store, spec=None):
        super.__init__(name, obj_api, opr_store, spec)
        self.table = []
        self.numchains = 0
        self.numchainreplicas = 0
        if spec is not None:
            self.set_obj_spec(spec)

    def get_obj_spec(self):
        self.obj = {
            "status": self.status,
            "numchains": self.numchains,
            "numchainreplicas": self.numchainreplicas,
            "table": self.table
        }

        return self.obj

    def set_obj_spec(self, spec):
        # K8s APIs
        self.status = get_spec_val('status', spec)
        self.numchains = get_spec_val('numchains', spec)
        self.numchainreplicas = get_spec_val('numchainreplicas', spec)
        self.table = get_spec_val('table', spec)

    def get_plural(self):
        return "dft"

    def get_kind(self):
        return "Dft"

    def store_update_obj(self):
        if self.store is None:
            return
        self.store.update_dft(self)

    def store_delete_obj(self):
        if self.store is None:
            return
        self.store.delete_dft(self.name)
