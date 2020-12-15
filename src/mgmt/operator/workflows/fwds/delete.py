# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from common.workflow import *
from operators.fwds_operator import *
from operators.droplets_operator import *

fwds_opr = FwdOperator()
droplets_opr = DropletOperator()


def fwd_delete(task, fwd, name, body, spec):
    logger.info("Deleting Fwd {}!".format(name))
    if not fwd:
        fwd = fwds_opr.get_stored_obj(name, spec)
    droplets_opr.unassign_droplet(fwd)
    return fwd
