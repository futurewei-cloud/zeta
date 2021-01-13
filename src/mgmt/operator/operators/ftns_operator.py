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
from operators.droplets_operator import *

droplets_opr = DropletOperator()
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

    def create_n_ftns(self, chain_obj, size, task):
        avail_droplets = len(
            droplets_opr.get_network_unallocated_droplets(OBJ_DEFAULTS.zgc_net))
        if avail_droplets < size:
            task.raise_temporary_error(
                "Not enough droplets available for {} FTN(s), only {} droplets available".format(size, avail_droplets))
        for i in range(size):
            ftn_id = self.id_allocator.allocate_id(KIND.fwd)
            ftn_name = chain_obj.name + '-ftn-' + ftn_id
            ftn_obj = Ftn(ftn_name, self.obj_api, self.store)
            if not droplets_opr.assign_droplet(ftn_obj, OBJ_DEFAULTS.zgc_net):
                task.raise_permanent_error(
                    "Unable to assign droplet! {} of {} FTN(s) assigned. {} available droplets.".format(i, size, avail_droplets))
            ftn_obj.id = ftn_id
            ftn_obj.parent_chain = chain_obj.name
            ftn_obj.dft = chain_obj.dft
            logger.info("Adding FTN {}".format(ftn_obj.name))
            chain_obj.ftns.insert(0, ftn_obj.name)
            ftn_obj.create_obj()
        # Update all FTNs in chain
        self.update_chain_ftns(chain_obj)

    def delete_n_ftns(self, chain_obj, numchainreplicas, task):
        if numchainreplicas > len(chain_obj.ftns):
            logger.info("Can't delete more FTNs than available.")
            chain_obj.size = len(chain_obj.ftns)
            return
        for i in range(numchainreplicas):
            ftn_obj = self.store.get_obj(chain_obj.ftns[i], KIND.ftn)
            logger.info("Deleting FTN {}".format(ftn_obj.name))
            chain_obj.ftns.remove(ftn_obj.name)
            ftn_obj.delete_obj()
        # Update all FTNs in chain
        self.update_chain_ftns(chain_obj)

    def process_numchainreplicas_change(self, chain, old, new, task):
        logger.info("New {}, Old {}".format(new, old))
        diff = new - old
        if diff > 0:
            logger.info("Scaling out ftns per chain: {}".format(abs(diff)))
            self.create_n_ftns(chain, abs(diff), task)

        if diff <= 0:
            logger.info("Scaling in ftns per chain: {}".format(abs(diff)))
            self.delete_n_ftns(chain, abs(diff), task)
        chain.update_obj()

    def update_chain_ftns(self, chain_obj):
        for ftn_name in chain_obj.ftns:
            ftn_obj = self.store.get_obj(ftn_name, KIND.ftn)
            ftn_obj.update_obj()
