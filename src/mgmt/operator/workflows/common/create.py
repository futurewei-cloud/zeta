# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from common.object_operator import ObjectOperator
from common.constants import KIND

logger = logging.getLogger()
objs_opr = ObjectOperator()


class CommonCreate(WorkflowTask):

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        obj = objs_opr.store_get_obj(
            self.param.name, self.param.body["kind"], self.param.spec)
        obj = self.param.workflow_func(self, obj, self.param.name,
                                       self.param.body, self.param.spec)
        if obj.kind != KIND.dft:
            obj.id = objs_opr.id_allocator.allocate_id(obj.name)
        objs_opr.set_object_provisioned(obj)
        self.finalize()
