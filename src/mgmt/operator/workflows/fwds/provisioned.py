# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from common.workflow import *
from operators.fwds_operator import *

fwds_opr = FwdOperator()


def fwd_provisioned(task, fwd, name, body, spec):
    logger.info("Provisioned Fwd {}!".format(name))
    return fwd
