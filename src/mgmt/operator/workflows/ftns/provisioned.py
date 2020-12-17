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

    # Derive FTN position
    ftn_index = chain_obj.ftns.index(ftn.name)
    next_ftn_index = ftn_index + 1
    if next_ftn_index == chain_obj.size:  # Tail or Single FTN chain
        droplet_obj.rpc.update_ftn(ftn.id, droplet_obj, None, 2)
    elif ftn_index == 0:  # Head
        next_ftn_obj = chains_opr.store.get_obj(
            chain_obj.ftns[next_ftn_index], KIND.ftn)
        next_ftn_droplet_obj = droplets_opr.store.get_obj(
            next_ftn_obj.droplet, KIND.droplet)
        droplet_obj.rpc.update_ftn(
            ftn.id, droplet_obj, next_ftn_droplet_obj, 0)
    else:  # Middle
        next_ftn_obj = chains_opr.store.get_obj(
            chain_obj.ftns[next_ftn_index], KIND.ftn)
        next_ftn_droplet_obj = droplets_opr.store.get_obj(
            next_ftn_obj.droplet, KIND.droplet)
        droplet_obj.rpc.update_ftn(
            ftn.id, droplet_obj, next_ftn_droplet_obj, 1)
    return ftn
