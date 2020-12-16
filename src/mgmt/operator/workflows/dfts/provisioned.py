# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from operators.dfts_operator import *
from operators.fwds_operator import *
from operators.droplets_operator import *
from operators.chains_operator import *

fwds_opr = FwdOperator()
dfts_opr = DftOperator()
droplets_opr = DropletOperator()
chains_opr = ChainOperator()


def dft_provisioned(task, dft, name, body, spec, diff):
    logger.info("Provisioned DFT {}!".format(name))
    if not dft:
        dft = dfts_opr.get_stored_obj(name, spec)

    for d in diff:
        if d[0] == 'change':
            process_change(dft=dft, field=d[1], old=d[2], new=d[3])

    # Update all fwd droplets with dft's maglev table
    for fwd in dft.fwds:
        fwd_obj = fwds_opr.store.get_obj(fwd, KIND.fwd)
        droplet = droplets_opr.store.get_obj(fwd_obj.droplet, KIND.droplet)
        droplet.rpc.update_dft(dft)

    return dft


def process_change(dft, field, old, new):
    if field[0] == 'spec' and field[1] == 'numchains':
        logger.info("diff_field:{}, from:{}, to:{}".format(field, old, new))
        return chains_opr.process_numchain_change(dft, int(old), int(new))
