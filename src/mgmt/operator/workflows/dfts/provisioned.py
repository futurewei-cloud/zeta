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

    logger.info("DIFF HERE {}".format(diff))
    logger.info("numchains {} numfwds {}".format(dft.numchains, dft.numfwds))
    for d in diff:
        if d[0] == 'change':
            logger.info(
                "diff_field:{}, from:{}, to:{}".format(d[1], d[2], d[3]))
            process_change(dft=dft, field=d[1], old=d[2], new=d[3], task=task)

    # Update all fwd droplets with dft's maglev table
    for fwd in dft.fwds:
        fwd_obj = fwds_opr.store.get_obj(fwd, KIND.fwd)
        droplet = droplets_opr.store.get_obj(fwd_obj.droplet, KIND.droplet)
        if not droplet:
            task.raise_temporary_error("FWD droplet not ready!")
        droplet.rpc.update_dft(dft)
    return dft


def process_change(dft, field, old, new, task):
    if field[0] == 'spec':
        if field[1] == 'numchains':
            logger.info(
                "Chain: diff_field:{}, from:{}, to:{}".format(field, old, new))
            chains_opr.process_numchain_change(dft, int(old), int(new), task)
        elif field[1] == 'numfwds':
            logger.info(
                "FWD: diff_field:{}, from:{}, to:{}".format(field, old, new))
            fwds_opr.process_numfwd_change(dft, int(old), int(new), task)
