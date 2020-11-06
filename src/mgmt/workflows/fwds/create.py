# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from common.workflow import *
from operators.fwds_operator import *
from operators.chains_operator import *
from operators.dfts_operator import *

dfts_opr = DftOperator()
fwds_opr = FwdOperator()


def fwd_create(task, fwd, name, body, spec):
    logger.info("Creating Fwd {}!".format(name))
    if not fwd:
        fwd = fwds_opr.get_stored_obj(name, spec)
    if not dfts_opr.store.contains_obj(fwd.dft, KIND.dft):
        task.raise_temporary_error(
            "DFT {} not yet created!".format(fwd.dft))
    return fwd
