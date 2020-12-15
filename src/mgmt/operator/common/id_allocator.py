# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
logger = logging.getLogger()


class IdAllocator:
    """
    Allocate unique IDs
    """
    _instance = None

    def __new__(cls, **kwargs):
        if cls._instance is None:
            cls._instance = super(IdAllocator, cls).__new__(cls)
            cls._init(cls, **kwargs)
        return cls._instance

    def _init(self, **kwargs):
        self.ids = []
        for i in range(1, 10000):  # id 0 reserved for DFT
            self.ids.append(str(i))

    def allocate_id(self, obj_name):
        logger.info("Allocating ID {} for {}!".format(self.ids[0], obj_name))
        return (self.ids.pop(0))

    def reclaim_id(self, obj_id, obj_name):
        logger.info("Reclaiming ID {} from {}!".format(obj_id, obj_name))
        self.ids.insert(0, obj_id)
