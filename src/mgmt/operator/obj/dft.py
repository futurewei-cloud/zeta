# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import json
from common.constants import *
from common.common import *
from common.kube_obj import KubeObject


class Dft(KubeObject):

    def __init__(self, name, obj_api, opr_store, spec=None):
        super().__init__(name, obj_api, opr_store, spec)
        self.kind = "Dft"
        self.plural = "dfts"
        self.maglev_table = None
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
