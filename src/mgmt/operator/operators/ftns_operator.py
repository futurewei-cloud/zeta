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
from obj.ftn import Ftn
from kubernetes import client, config

logger = logging.getLogger()


class FtnOperator(ObjectOperator):
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

    def query_existing_ftns(self):
        def list_ftn_obj_fn(name, spec, plurals):
            logger.info("Bootstrapped ftn {}".format(name))
            ftn = Ftn(name, self.obj_api, self.store, spec)
            if ftn.status == OBJ_STATUS.obj_status_provisioned:
                self.store.update_obj(ftn)

        kube_list_obj(self.obj_api, RESOURCES.ftns, list_ftn_obj_fn)
        self.bootstrapped = True

    def get_stored_obj(self, name, spec):
        return Ftn(name, self.obj_api, self.store, spec)

    def create_n_ftns(self, chain, size):
        for i in range(size):
            ftn_name = chain.name + '-ftn-' + str(i)
            ftn = Ftn(ftn_name, self.obj_api, self.store)
            ftn.parent_chain = chain.name
            ftn.dft = chain.dft
            ftn.id = self.id_allocator.allocate_id(ftn.name)
            chain.ftns.append(ftn.name)
            ftn.create_obj()
        chain.tail = chain.ftns[-1]
        chain.head = chain.ftns[0]

    def delete_n_ftns(self, chain, numchainreplicas):
        if numchainreplicas > len(chain.size):
            logger.info("Can't delete more FTNs than available.")
            return
        for i in range(numchainreplicas):
            ftn_obj = self.store.get_obj(chain.ftns[i], KIND.ftn)
            ftn_obj.delete_obj()

    def process_numchainreplicas_change(self, chain, old, new):
        diff = new - old
        if diff > 0:
            logger.info("Scaling out ftns per chain: {}".format(abs(diff)))
            self.create_n_ftns(chain, abs(diff))

        if diff < 0:
            logger.info("Scaling in ftns per chain: {}".format(abs(diff)))
            self.delete_n_ftns(chain, abs(diff))
