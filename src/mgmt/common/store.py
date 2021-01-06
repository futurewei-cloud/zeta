# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: Zeta-manager App
#
import logging
import inspect

logger = logging.getLogger()

class Store(object):

    def __init__(self, **kwargs):
        logger.info(kwargs)
        self.store = {}

    def update_obj(self, obj):
        self.store[obj.name] = obj

    def get_obj(self, name):
        if name in self.store[kind]:
            return self.store[kind][name]
        logger.info(
            "Object {} not found".format(name))
        return None

    def delete_obj(self, name):
        if name in self.store:
            del self.store[name]
        else:
            logger.info(
                "Object {} not found".format(name))

    def get_all_obj_type(self):
        return self.store.keys()

    def contains_obj(self, name):
        if name in self.store:
            return True
        return False

    def dump_obj(self):
        for obj in self.store.values():
            logger.info("{}, Spec: {}".format(
                obj.name, obj.get_obj_spec()))
