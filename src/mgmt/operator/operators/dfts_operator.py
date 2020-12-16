# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import random
from kubernetes import client, config
from common.constants import *
from common.common import *
from common.object_operator import ObjectOperator
from common.id_allocator import IdAllocator
from obj.dft import Dft
from store.operator_store import OprStore
from common.maglev_table import MaglevTable

logger = logging.getLogger()


class DftOperator(ObjectOperator):
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

    def query_existing_dfts(self):
        def list_dft_obj_fn(name, spec, plurals):
            logger.info("Bootstrapped dft {}".format(name))
            dft = Dft(name, self.obj_api, self.store, spec)
            if dft.status == OBJ_STATUS.obj_status_provisioned:
                self.store.update_obj(dft)

        kube_list_obj(self.obj_api, RESOURCES.dfts, list_dft_obj_fn)
        self.bootstrapped = True

    def get_stored_obj(self, name, spec):
        return Dft(name, self.obj_api, self.store, spec)

    def create_default_dft(self):
        if self.store.get_obj(OBJ_DEFAULTS.default_dft, KIND.dft):
            return
        dft = Dft(OBJ_DEFAULTS.default_dft, self.obj_api, self.store)
        dft.numchains = OBJ_DEFAULTS.default_n_chains
        dft.maglev_table = MaglevTable(OBJ_DEFAULTS.default_maglev_table_size)
        dft.create_obj()
