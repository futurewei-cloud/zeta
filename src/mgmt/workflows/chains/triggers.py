# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import kopf
import logging
import asyncio
from common.common import *
from common.constants import *
from common.wf_factory import *
from common.wf_param import *


@kopf.on.resume(group, version, RESOURCES.chains, when=LAMBDAS.status_init, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.update(group, version, RESOURCES.chains, when=LAMBDAS.status_init, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.create(group, version, RESOURCES.chains, when=LAMBDAS.status_init, retries=OBJ_DEFAULTS.kopf_max_retries)
def chain_opr_on_chain_init(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    run_workflow(wffactory().ChainCreate(param=param))


@kopf.on.resume(group, version, RESOURCES.chains, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.update(group, version, RESOURCES.chains, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.create(group, version, RESOURCES.chains, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
def chain_opr_on_chain_provisioned(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    run_workflow(wffactory().ChainProvisioned(param=param))


@kopf.on.delete(group, version, RESOURCES.chains, retries=OBJ_DEFAULTS.kopf_max_retries)
def chain_opr_on_chain_delete(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    run_workflow(wffactory().ChainDelete(param=param))
