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
    return chain
