# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from common.workflow import *
from operators.ftns_operator import *
from operators.droplets_operator import *
from operators.chains_operator import *

ftns_opr = FtnOperator()
droplets_opr = DropletOperator()
chains_opr = ChainOperator()


def ftn_delete(task, ftn, name, body, spec, diff):
    logger.info("Deleting Ftn {}!".format(name))
    if not ftn:
        ftn = ftns_opr.get_stored_obj(name, spec)
    droplet_obj = droplets_opr.store.get_obj(ftn.droplet, KIND.droplet)
    droplet_obj.rpc.delete_ftn(ftn)
    droplets_opr.unassign_droplet(ftn)
    return ftn
