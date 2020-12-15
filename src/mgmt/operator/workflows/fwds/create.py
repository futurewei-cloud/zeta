# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from common.workflow import *
from operators.fwds_operator import *
from operators.chains_operator import *
from operators.dfts_operator import *
from operators.droplets_operator import *
from common.constants import OBJ_DEFAULTS

dfts_opr = DftOperator()
fwds_opr = FwdOperator()
droplets_opr = DropletOperator()


def fwd_create(task, fwd, name, body, spec):
    logger.info("Creating Fwd {}!".format(name))
    if not fwd:
        fwd = fwds_opr.get_stored_obj(name, spec)
    if not dfts_opr.store.contains_obj(fwd.dft, KIND.dft):
        task.raise_temporary_error(
            "DFT {} not yet created!".format(fwd.dft))
    if len(droplets_opr.get_unallocated_droplets()) < 1:
        task.raise_temporary_error(
            "No droplets available for FWD")
    droplets_opr.assign_droplet(fwd, OBJ_DEFAULTS.tenant_net)
    return fwd
