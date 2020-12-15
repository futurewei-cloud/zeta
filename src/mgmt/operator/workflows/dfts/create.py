# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from operators.dfts_operator import *
from operators.chains_operator import *
from operators.droplets_operator import *

dfts_opr = DftOperator()
chains_opr = ChainOperator()
droplets_opr = DropletOperator()


def dft_create(task, dft, name, body, spec):
    logger.info("Creating DFT {}!".format(name))
    if not dft:
        dft = dfts_opr.get_stored_obj(name, spec)
    if len(droplets_opr.get_unallocated_droplets()) < 1:
        task.raise_temporary_error(
            "No droplets available for DFT")
    chains_opr.create_n_chains(dft)
    logger.info("DFT table: {}".format(dft.table))
    return dft
