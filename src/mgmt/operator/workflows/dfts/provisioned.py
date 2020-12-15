# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from operators.dfts_operator import *
from operators.fwds_operator import *
from operators.droplets_operator import *

fwds_opr = FwdOperator()
dfts_opr = DftOperator()
droplets_opr = DropletOperator()


def dft_provisioned(task, dft, name, body, spec):
    logger.info("Provisioned DFT {}!".format(name))
    if not dft:
        dft = dfts_opr.get_stored_obj(name, spec)

    # Update all fwd droplets with dft's maglev table
    for fwd in dft.fwds:
        fwd_obj = fwds_opr.store.get_obj(fwd, KIND.fwd)
        droplet = droplets_opr.store.get_obj(fwd_obj.droplet, KIND.droplet)
        droplet.rpc.update_dft(dft)
    return dft
