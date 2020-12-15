# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from operators.chains_operator import *

logger = logging.getLogger()
chains_opr = ChainOperator()


def chain_provisioned(task, chain, name, body, spec):
    logger.info("Provisioned Chain {}!".format(name))
    if not chain:
        chain = chains_opr.get_stored_obj(name, spec)
    return chain
