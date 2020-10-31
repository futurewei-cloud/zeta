# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from operators.ftns_operator import *

logger = logging.getLogger()
ftns_opr = FtnOperator()


class FtnDelete(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        ftn = ftns_opr.store_get_obj(
            self.param.name, self.param.spec["plural"], self.param.spec)

        ftns_opr.store_delete_obj(ftn.name, ftn.resource)
        self.finalize()
