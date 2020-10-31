# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import random
from common.constants import *
from common.common import *
from common.object_operator import ObjectOperator
from kubernetes import client, config
from obj.dft import Dft
from store.operator_store import OprStore

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
