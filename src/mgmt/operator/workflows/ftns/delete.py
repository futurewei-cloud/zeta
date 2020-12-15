# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from common.workflow import *
from operators.ftns_operator import *
from operators.droplets_operator import *

ftns_opr = FtnOperator()
droplets_opr = DropletOperator()


def ftn_delete(task, ftn, name, body, spec):
    logger.info("Deleting Ftn {}!".format(name))
    if not ftn:
        ftn = ftns_opr.get_stored_obj(name, spec)
    droplets_opr.unassign_droplet(ftn)
    return ftn
