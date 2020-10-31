# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from operators.dfts_operator import *
from operators.chains_operator import *

logger = logging.getLogger()
dfts_opr = DftOperator()


class DftCreate(WorkflowTask):

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        dft = dfts_opr.store_get_obj(
            self.param.name, self.param.spec["plural"], self.param.spec)

        dfts_opr.set_object_provisioned(dft)
        self.finalize()
