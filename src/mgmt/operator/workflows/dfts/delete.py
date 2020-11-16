# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from operators.dfts_operator import *

dfts_opr = DftOperator()


def dft_delete(task, dft, name, body, spec):
    logger.info("Deleting DFT {}!".format(name))
    return dft
