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
import json
from common.rpc import TrnRpc
from common.constants import *
from common.common import *
from common.common import run_cmd
from common.kube_obj import KubeObject

logger = logging.getLogger()


class Droplet(KubeObject):

    def __init__(self, name, obj_api, opr_store, spec=None):
        super().__init__(name, obj_api, opr_store, spec)
        self.kind = "Droplet"
        self.plural = "droplets"
        self.ip = []
        self.mac = []
        self.ip_control = ""
        self.zgc_id = ""
        self.phy_itf = ""
        self.network = ""
        if spec is not None:
            self.set_obj_spec(spec)

    @property
    def rpc(self):
        return TrnRpc(self.ip)

    def get_obj_spec(self):
        self.obj = {
            "mac": self.mac,
            "ip": self.ip,
            "status": self.status,
            "itf": self.phy_itf,
            "network": self.network,
            "zgc_id": self.zgc_id,
            "ip_control": self.ip_control
        }

        return self.obj

    def set_obj_spec(self, spec):
        self.status = get_spec_val('status', spec)
        self.mac = get_spec_val('mac', spec)
        self.ip = get_spec_val('ip', spec)
        self.phy_itf = get_spec_val('itf', spec)
        self.network = get_spec_val('network', spec)
        self.zgc_id = get_spec_val('zgc_id', spec)
        self.ip_control = get_spec_val('ip_control', spec)
