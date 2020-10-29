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
        ftn = ftns_opr.store.get_ftn(self.param.name)
        ftns_opr.store.delete_ftn(ftn.name)
        ftn.delete_obj()
        self.finalize()
