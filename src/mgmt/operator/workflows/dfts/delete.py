# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from operators.dfts_operator import *
from operators.chains_operator import *
from operators.droplets_operator import *
from operators.fwds_operator import *

dfts_opr = DftOperator()
chains_opr = ChainOperator()
droplets_opr = DropletOperator()
fwds_opr = FwdOperator()


def dft_delete(task, dft, name, body, spec, diff):
    logger.info("Deleting DFT {}!".format(name))
    if not dft:
        dft = dfts_opr.get_stored_obj(name, spec)

    # Delete all chains
    for chain in dft.chains:
        chain_obj = chains_opr.store.get_obj(chain, KIND.chain)
        chain_obj.delete_obj()
    # Delete all fwds
    for fwd in dft.fwds:
        fwd_obj = fwds_opr.store.get_obj(fwd, KIND.fwd)
        fwd_obj.delete_obj()
    return dft
