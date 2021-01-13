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
from operators.droplets_operator import *

droplets_opr = DropletOperator()
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

    def create_n_fwds(self, dft_obj, numfwds, task):
        avail_droplets = len(
            droplets_opr.get_network_unallocated_droplets(OBJ_DEFAULTS.tenant_net))
        if avail_droplets < numfwds:
            task.raise_temporary_error(
                "Not enough droplets available for {} FWD(s), only {} available droplets.".format(numfwds, avail_droplets))
        for _ in range(numfwds):
            fwd_id = self.id_allocator.allocate_id(KIND.fwd)
            fwd_name = dft_obj.name + '-fwd-' + fwd_id
            fwd_obj = Fwd(fwd_name, self.obj_api, self.store)
            if not droplets_opr.assign_droplet(fwd_obj, OBJ_DEFAULTS.tenant_net):
                task.raise_permanent_error(
                    "Unable to assign droplet! {} out of {} FWD(s) assigned. {} available droplets.".format(i, numfwds, avail_droplets))
            fwd_obj.id = fwd_id
            fwd_obj.dft = dft_obj.name
            logger.info("Adding FWD {}".format(fwd_obj.name))
            dft_obj.fwds.append(fwd_obj.name)
            fwd_obj.create_obj()

    def delete_n_fwds(self, dft_obj, numfwds, task):
        if numfwds > len(dft_obj.fwds):
            logger.info("Can't delete more FWDs than available.")
            dft_obj.numfwds = len(dft_obj.fwds)
        for i in range(numfwds):
            fwd_obj = self.store.get_obj(dft_obj.fwds[i], KIND.fwd)
            logger.info("Deleting FWD {}".format(fwd_obj.name))
            dft_obj.fwds.remove(fwd_obj.name)
            fwd_obj.delete_obj()

    def process_numfwd_change(self, dft_obj, old, new, task):
        logger.info("New {}, Old {}".format(new, old))
        diff = new - old
        if diff > 0:
            logger.info("Scaling out FWDs by {}".format(abs(diff)))
            self.create_n_fwds(
                dft_obj, abs(diff), task)
        if diff <= 0:
            logger.info("Scaling in FWDs by {}".format(abs(diff)))
            self.delete_n_fwds(dft_obj, abs(diff), task)
        dft_obj.update_obj()
