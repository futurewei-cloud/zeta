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
        self.id = OBJ_DEFAULTS.default_dft_id
        self.kind = "Dft"
        self.plural = "dfts"
        self.maglev_table = None
        self.table = []
        self.numchains = 0
        self.chains = []
        self.fwds = []
        self.id = OBJ_DEFAULTS.default_dft_id
        if spec is not None:
            self.set_obj_spec(spec)

    def get_obj_spec(self):
        self.obj = {
            "id": self.id,
            "status": self.status,
            "numchains": self.numchains,
            "table": self.table,
            "chains": self.chains,
            "fwds": self.fwds
        }

        return self.obj

    def set_obj_spec(self, spec):
        # K8s APIs
        self.id = get_spec_val('id', spec)
        self.status = get_spec_val('status', spec)
        self.numchains = get_spec_val('numchains', spec)
        self.table = get_spec_val('table', spec)
        self.fwds = get_spec_val('fwds', spec)
        self.chains = get_spec_val('chains', spec)
