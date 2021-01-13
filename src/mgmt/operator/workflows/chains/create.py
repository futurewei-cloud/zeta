# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from operators.chains_operator import *
from operators.dfts_operator import *
from operators.ftns_operator import *

logger = logging.getLogger()
chains_opr = ChainOperator()
dfts_opr = DftOperator()
ftns_opr = FtnOperator()


def chain_create(task, chain, name, body, spec, diff):
    logger.info("Creating Chain {}!".format(name))
    if not chain:
        chain = chains_opr.get_stored_obj(name, spec)
    if not dfts_opr.store.contains_obj(chain.dft, KIND.dft):
        task.raise_temporary_error(
            "Parent DFT {} not yet created!".format(chain.dft))
    if chain.size < 1:
        task.raise_permanent_error("Chain size cannot be less than 1")
    ftns_opr.create_n_ftns(chain, chain.size, task)
    return chain
