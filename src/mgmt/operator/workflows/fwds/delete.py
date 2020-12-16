# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from common.workflow import *
from operators.chains_operator import *
from operators.droplets_operator import *
from operators.dfts_operator import *
from operators.ftns_operator import *
from operators.fwds_operator import *

dfts_opr = DftOperator()
chains_opr = ChainOperator()
ftns_opr = FtnOperator()
fwds_opr = FwdOperator()
droplets_opr = DropletOperator()


def fwd_delete(task, fwd, name, body, spec, diff):
    logger.info("Deleting Fwd {}!".format(name))
    if not fwd:
        fwd = fwds_opr.get_stored_obj(name, spec)

    dft_obj = dfts_opr.store.get_obj(fwd.dft, KIND.dft)
    fwd_droplet_obj = droplets_opr.store.get_obj(fwd.droplet, KIND.droplet)

    # Delete DFT hash table, chain information from FWD
    for chain in dft_obj.chains:
        chain_obj = chains_opr.store.get_obj(chain, KIND.chain)
        fwd_droplet_obj.rpc.delete_chain(chain_obj)
    fwd_droplet_obj.rpc.delete_dft(dft_obj)

    droplets_opr.unassign_droplet(fwd)
    return fwd
