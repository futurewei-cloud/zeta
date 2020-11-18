# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from common.workflow import *
from operators.ftns_operator import *

ftns_opr = FtnOperator()


def ftn_provisioned(task, ftn, name, body, spec):
    logger.info("Provisioned Ftn {}!".format(name))
    return ftn
