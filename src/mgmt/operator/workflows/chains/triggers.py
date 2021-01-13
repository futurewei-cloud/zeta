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
from workflows.chains.create import *
from workflows.chains.delete import *
from workflows.chains.provisioned import *


@kopf.on.resume(group, version, RESOURCES.chains, when=LAMBDAS.status_init, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.update(group, version, RESOURCES.chains, when=LAMBDAS.status_init, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.create(group, version, RESOURCES.chains, when=LAMBDAS.status_init, retries=OBJ_DEFAULTS.kopf_max_retries)
def chain_opr_on_chain_init(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    param.workflow_func = chain_create
    run_workflow(wffactory().CommonCreate(param=param))


@kopf.on.resume(group, version, RESOURCES.chains, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.update(group, version, RESOURCES.chains, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.create(group, version, RESOURCES.chains, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
def chain_opr_on_chain_provisioned(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    param.diff = kwargs['diff']
    param.workflow_func = chain_provisioned
    run_workflow(wffactory().CommonProvisioned(param=param))


@kopf.on.delete(group, version, RESOURCES.chains, retries=OBJ_DEFAULTS.kopf_max_retries)
def chain_opr_on_chain_delete(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    param.workflow_func = chain_delete
    run_workflow(wffactory().CommonDelete(param=param))
