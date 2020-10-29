# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import random
from common.constants import *
from common.common import *
from store.operator_store import OprStore
from kubernetes import client, config


logger = logging.getLogger()


class ObjectOperator(object):

    def set_object_provisioned(self, obj):
        obj.set_status(OBJ_STATUS.obj_status_provisioned)
        obj.update_obj()
