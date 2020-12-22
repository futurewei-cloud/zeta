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
from store.operator_store import OprStore
from obj.chain import Chain

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
        self.id_allocator = IdAllocator()
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

    def create_n_chains(self, dft_obj, numchains, numchainreplicas, task):
        for _ in range(numchains):
            chain_id = self.id_allocator.allocate_id(KIND.chain)
            chain_name = dft_obj.name + '-chain-' + chain_id
            chain_obj = Chain(chain_name, self.obj_api, self.store)
            chain_obj.id = chain_id
            chain_obj.dft = dft_obj.name
            chain_obj.size = numchainreplicas
            logger.info("Adding chain {}".format(chain_obj.name))
            dft_obj.chains.append(chain_obj.name)
            dft_obj.maglev_table.add(chain_obj.id)
            chain_obj.create_obj()
        dft_obj.table = dft_obj.maglev_table.get_table()

    def delete_n_chains(self, dft_obj, numchains, task):
        if numchains > dft_obj.numchains:
            task.raise_permanent_error(
                "Can't delete more Chains than available.")
        for i in range(numchains):
            chain_obj = self.store.get_obj(dft_obj.chains[i], KIND.chain)
            logger.info("Deleting chain {}".format(chain_obj.name))
            dft_obj.chains.remove(chain_obj.name)
            dft_obj.maglev_table.remove(chain_obj.id)
            chain_obj.delete_obj()
        dft_obj.table = dft_obj.maglev_table.get_table()

    def process_numchain_change(self, dft_obj, old, new, task):
        logger.info("New {}, Old {}".format(new, old))
        diff = new - old
        if diff > 0:
            logger.info("Scaling out chain by {}".format(abs(diff)))
            self.create_n_chains(
                dft_obj, abs(diff), OBJ_DEFAULTS.default_n_ftns, task)
        if diff <= 0:
            logger.info("Scaling in chains by {}".format(abs(diff)))
            self.delete_n_chains(dft_obj, abs(diff), task)
        dft_obj.update_obj()
