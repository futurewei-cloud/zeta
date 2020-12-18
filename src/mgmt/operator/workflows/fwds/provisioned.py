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


def fwd_provisioned(task, fwd, name, body, spec, diff):
    logger.info("Provisioned Fwd {}!".format(name))
    if not fwd:
        fwd = fwds_opr.get_stored_obj(name, spec)
    dft_obj = dfts_opr.store.get_obj(fwd.dft, KIND.dft)
    fwd_droplet_obj = droplets_opr.store.get_obj(fwd.droplet, KIND.droplet)

    for chain in dft_obj.chains:
        chain_obj = chains_opr.store.get_obj(chain, KIND.chain)
        if len(chain_obj.ftns) > 0:
            tail_ftn_obj = ftns_opr.store.get_obj(chain_obj.ftns[-1], KIND.ftn)
            tail_ftn_droplet_obj = droplets_opr.store.get_obj(
                tail_ftn_obj.droplet, KIND.droplet)
            if not tail_ftn_droplet_obj:
                task.raise_temporary_error("FTN droplet not provisioned")
            fwd_droplet_obj.rpc.update_chain(
                chain_obj, tail_ftn_droplet_obj, tail_ftn_obj.id)
    fwd_droplet_obj.rpc.update_dft(dft_obj)
    return fwd
