# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from operators.chains_operator import *
from operators.dfts_operator import *
from operators.ftns_operator import *
from operators.fwds_operator import *
from operators.droplets_operator import *

logger = logging.getLogger()
chains_opr = ChainOperator()
dfts_opr = DftOperator()
ftns_opr = FtnOperator()
fwds_opr = FwdOperator()
droplets_opr = DropletOperator()


def chain_provisioned(task, chain, name, body, spec, diff):
    logger.info("Provisioned Chain {}!".format(name))
    if not chain:
        chain = chains_opr.get_stored_obj(name, spec)

    for d in diff:
        if d[0] == 'change':
            process_change(
                chain=chain, field=d[1], old=d[2], new=d[3], task=task)

    tail_ftn_obj = ftns_opr.store.get_obj(chain.ftns[-1], KIND.ftn)
    if not tail_ftn_obj:
        task.raise_temporary_error("Tail FTN not yet provisioned!")

    dft_obj = dfts_opr.store.get_obj(chain.dft, KIND.dft)
    # Update all Fwds about chain and its tail ftn
    for fwd in dft_obj.fwds:
        fwd_obj = fwds_opr.store.get_obj(fwd, KIND.fwd)
        droplet_obj = droplets_opr.store.get_obj(
            fwd_obj.droplet, KIND.droplet)
        tail_ftn_droplet_obj = droplets_opr.store.get_obj(
            tail_ftn_obj.droplet, KIND.droplet)
        if not tail_ftn_droplet_obj:
            task.raise_temporary_error("FTN droplet not provisioned")
        droplet_obj.rpc.update_chain(
            chain, tail_ftn_droplet_obj, tail_ftn_obj.id)
    return chain


def process_change(chain, field, old, new, task):
    if field[0] == 'spec':
        if field[1] == 'size':
            logger.info(
                "FTN: diff_field:{}, from:{}, to:{}".format(field, old, new))
            ftns_opr.process_numchainreplicas_change(
                chain, int(old), int(new), task)
