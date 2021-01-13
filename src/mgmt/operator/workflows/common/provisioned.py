# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from common.object_operator import ObjectOperator

logger = logging.getLogger()
objs_opr = ObjectOperator()


class CommonProvisioned(WorkflowTask):

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        obj = objs_opr.store_get_obj(
            self.param.name, self.param.body["kind"], self.param.spec)
        self.param.workflow_func(self, obj, self.param.name,
                                 self.param.body, self.param.spec, self.param.diff)
        objs_opr.store.update_obj(obj)
        self.finalize()
