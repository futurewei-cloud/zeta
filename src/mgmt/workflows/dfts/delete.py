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
        dft = dfts_opr.store.get_dft(self.param.name)
        dfts_opr.store.delete_dft(dft.name)
        dft.delete_obj()
        self.finalize()
