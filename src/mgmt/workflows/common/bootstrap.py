# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from operators.chains_operator import *
from operators.dfts_operator import *
from operators.ftns_operator import *
from operators.droplets_operator import *
logger = logging.getLogger()

chains_opr = ChainOperator()
ftns_opr = FtnOperator()
droplets_opr = DropletOperator()
dfts_opr = DftOperator()


class CommonOperatorStart(WorkflowTask):

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        droplets_opr.query_existing_droplets()
        ftns_opr.query_existing_ftns()
        chains_opr.query_existing_chains()
        dfts_opr.query_existing_dfts()
        self.finalize()
