# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import random
from common.constants import *
from common.common import *
from common.object_operator import ObjectOperator
from store.operator_store import OprStore
from common.id_allocator import IdAllocator
from obj.fwd import Fwd
from kubernetes import client, config

logger = logging.getLogger()


class FwdOperator(ObjectOperator):
    _instance = None

    def __new__(cls, **kwargs):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls.__init__(cls, **kwargs)
        return cls._instance

    def __init__(self, **kwargs):
        logger.info(kwargs)
        self.store = OprStore()
        self.id_allocator = IdAllocator()
        config.load_incluster_config()
        self.obj_api = client.CustomObjectsApi()

    def query_existing_fwds(self):
        def list_fwd_obj_fn(name, spec, plurals):
            logger.info("Bootstrapped fwd {}".format(name))
            fwd = Fwd(name, self.obj_api, self.store, spec)
            if fwd.status == OBJ_STATUS.obj_status_provisioned:
                self.store.update_obj(fwd)

        kube_list_obj(self.obj_api, RESOURCES.fwds, list_fwd_obj_fn)
        self.bootstrapped = True

    def get_stored_obj(self, name, spec):
        return Fwd(name, self.obj_api, self.store, spec)

    def create_default_fwds(self, n, default_dft):
        for i in range(n):
            fwd_name = default_dft + '-fwd-' + str(i)
            fwd = Fwd(fwd_name, self.obj_api, self.store)
            fwd.dft = default_dft
            fwd.create_obj()
