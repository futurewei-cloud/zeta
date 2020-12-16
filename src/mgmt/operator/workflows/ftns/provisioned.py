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


def ftn_provisioned(task, ftn, name, body, spec, diff):
    logger.info("Provisioned Ftn {}!".format(name))
    if not ftn:
        ftn = ftns_opr.get_stored_obj(name, spec)
    chain_obj = chains_opr.store.get_obj(ftn.parent_chain, KIND.chain)
    droplet_obj = droplets_opr.store.get_obj(ftn.droplet, KIND.droplet)
    ftn_index = chain_obj.ftns.index(ftn.name)
    next_ftn_index = ftn_index + 1

    # Derive FTN position
    if next_ftn_index == chain_obj.size:  # FTN is tail
        droplet_obj.update_ftn(ftn, None, 2)
    elif ftn_index == 0:  # FTN is head
        next_ftn_obj = chains_opr.store.get_obj(
            chain_obj.ftns[next_ftn_index], KIND.ftn)
        droplet_obj.update_ftn(ftn, next_ftn_obj, 0)
    else:  # FTN is middle
        next_ftn_obj = chains_opr.store.get_obj(
            chain_obj.ftns[next_ftn_index], KIND.ftn)
        droplet_obj.update_ftn(ftn, next_ftn_obj, 1)
    return ftn
