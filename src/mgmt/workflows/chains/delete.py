# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from operators.chains_operator import *

logger = logging.getLogger()
chains_opr = ChainOperator()


def chain_delete(task, chain, name, body, spec):
    logger.info("Deleting Chain {}!".format(name))
    return chain
