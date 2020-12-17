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


def chain_delete(task, chain, name, body, spec, diff):
    logger.info("Deleting Chain {}!".format(name))
    if not chain:
        chain = chains_opr.get_stored_obj(name, spec)

    # Delete chain from parent DFT table, generate maglev, update kubernetes dft obj
    dft_obj = dfts_opr.store.get_obj(chain.dft, KIND.dft)
    if dft_obj:  # Single chain deleted
        # Delete chain information from all FWDs
        for fwd in dft_obj.fwds:
            fwd_obj = fwds_opr.store.get_obj(fwd, KIND.fwd)
            droplet_obj = droplets_opr.store.get_obj(
                fwd_obj.droplet, KIND.droplet)
            droplet_obj.rpc.delete_chain(chain)
    for ftn_name in chain.ftns:
        ftn_obj = ftns_opr.store.get_obj(ftn_name, KIND.ftn)
        ftn_obj.delete_obj()
    return chain
