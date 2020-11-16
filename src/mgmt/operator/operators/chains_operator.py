# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import random
from common.constants import *
from common.common import *
from common.object_operator import ObjectOperator
from store.operator_store import OprStore
from obj.chain import Chain
from kubernetes import client, config

logger = logging.getLogger()


class ChainOperator(ObjectOperator):
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

    def query_existing_chains(self):
        def list_chain_obj_fn(name, spec, plurals):
            logger.info("Bootstrapped chain {}".format(name))
            chain = Chain(name, self.obj_api, self.store, spec)
            if chain.status == OBJ_STATUS.obj_status_provisioned:
                self.store.update_obj(chain)

        kube_list_obj(self.obj_api, RESOURCES.chains, list_chain_obj_fn)
        self.bootstrapped = True

    def get_stored_obj(self, name, spec):
        return Chain(name, self.obj_api, self.store, spec)
