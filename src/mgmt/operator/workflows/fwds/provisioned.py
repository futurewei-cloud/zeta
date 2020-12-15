# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from common.workflow import *
from operators.fwds_operator import *
from operators.dfts_operator import *
from operators.droplets_operator import *

fwds_opr = FwdOperator()
dfts_opr = DftOperator()
droplets_opr = DropletOperator()


def fwd_provisioned(task, fwd, name, body, spec):
    logger.info("Provisioned Fwd {}!".format(name))
    if not fwd:
        fwd = fwds_opr.get_stored_obj(name, spec)
    dft_obj = dfts_opr.store.get_obj(fwd.dft, KIND.dft)
    droplet_obj = droplets_opr.store.get_obj(fwd.droplet, KIND.droplet)
    droplet_obj.rpc.update_dft(dft_obj)
    return fwd
