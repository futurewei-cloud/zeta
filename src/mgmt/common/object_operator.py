# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.constants import *
from store.operator_store import OprStore
from common.id_allocator import IdAllocator

logger = logging.getLogger()


class ObjectOperator(object):

    def __init__(self, **kwargs):
        logger.info(kwargs)
        self.store = OprStore()
        self.id_allocator = IdAllocator()

    def store_get_obj(self, name, kind, spec):
        obj = self.store.get_obj(name, kind)
        if not obj:
            logger.info(
                "{} of type {} not found in store".format(name, kind))
            return None
        obj.set_obj_spec(spec)
        return obj

    def store_update_obj(self, obj):
        self.store.update_obj(obj)
        obj.update_obj()

    def store_delete_obj(self, obj):
        self.store.delete_obj(obj.name, obj.kind)
        obj.delete_obj()

    def set_object_provisioned(self, obj):
        obj.set_status(OBJ_STATUS.obj_status_provisioned)
        self.store_update_obj(obj)
