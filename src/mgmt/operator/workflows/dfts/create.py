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


def dft_create(task, dft, name, body, spec, diff):
    logger.info("Creating DFT {}!".format(name))
    if not dft:
        dft = dfts_opr.get_stored_obj(name, spec)
    if len(droplets_opr.get_unallocated_droplets()) < 1:
        task.raise_temporary_error(
            "No droplets available for DFT")
    if dft.numchains < 0:
        task.raise_permanent_error("DFT numchains cannot be less than 0")
    chains_opr.create_n_chains(
        dft, dft.numchains, OBJ_DEFAULTS.default_n_ftns, task)
    fwds_opr.create_n_fwds(
        dft, dft.numfwds, task)
    logger.info("DFT table: {}".format(dft.table))
    return dft
