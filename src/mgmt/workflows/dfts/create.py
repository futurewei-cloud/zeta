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

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        dft = dfts_opr.get_dft_stored_obj(self.param.name)
        dft.set_obj_spec(self.param.spec)
        dfts_opr.set_object_provisioned(dft)
        dfts_opr.store_update(dft)
        self.finalize()
