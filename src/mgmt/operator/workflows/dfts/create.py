# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from operators.dfts_operator import *
from operators.chains_operator import *

dfts_opr = DftOperator()
chains_opr = ChainOperator()


def dft_create(task, dft, name, body, spec):
    logger.info("Creating DFT {}!".format(name))
    if not dft:
        dft = dfts_opr.get_stored_obj(name, spec)
    chains_opr.create_n_chains(dft)
    return dft
