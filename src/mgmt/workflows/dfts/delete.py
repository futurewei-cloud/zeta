# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from operators.dfts_operator import *

logger = logging.getLogger()
dfts_opr = DftOperator()


class DftDelete(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        dft = dfts_opr.store_get_obj(
            self.param.name, self.param.spec["plural"], self.param.spec)

        dfts_opr.store_delete_obj(dft.name, dft.resource)
        self.finalize()
