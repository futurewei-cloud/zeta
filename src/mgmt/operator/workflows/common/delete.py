# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from common.object_operator import ObjectOperator
from common.constants import KIND

logger = logging.getLogger()
objs_opr = ObjectOperator()


class CommonDelete(WorkflowTask):

    def run(self):
        logger.info("Run {task}".format(task=self.__class__.__name__))
        obj = objs_opr.store_get_obj(
            self.param.name, self.param.body["kind"], self.param.spec)
        objs_opr.store.delete_obj(obj.name, obj.kind)
        obj = self.param.workflow_func(self, obj, self.param.name,
                                       self.param.body, self.param.spec, self.param.diff)
        if obj.kind != KIND.dft:
            objs_opr.id_allocator.reclaim_id(obj.id, obj.name)
        self.finalize()
