# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from operators.dfts_operator import *

dfts_opr = DftOperator()


def dft_create(dft, name, body, spec):
    logger.info("Creating DFT {}!".format(name))
    if not dft:
        dft = dfts_opr.get_stored_obj(name, spec)
    return dft
