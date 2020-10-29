# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from operators.dfts_operator import *
from operators.chains_operator import *

logger = logging.getLogger()
dfts_opr = DftOperator()
chains_opr = ChainOperator()


class ChainCreate(WorkflowTask):

    def requires(self):
        logger.info("Requires {task}".format(task=self.__class__.__name__))
        return []

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        chain = chains_opr.get_chain_stored_obj(self.param.name)
        chain.set_obj_spec(self.param.spec)
        chains_opr.set_object_provisioned(chain)
        chains_opr.store_update(chain)
        self.finalize()
