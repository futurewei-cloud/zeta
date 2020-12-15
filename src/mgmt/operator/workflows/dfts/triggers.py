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
from workflows.dfts.create import *
from workflows.dfts.delete import *
from workflows.dfts.provisioned import *


@kopf.on.resume(group, version, RESOURCES.dfts, when=LAMBDAS.status_init)
@kopf.on.update(group, version, RESOURCES.dfts, when=LAMBDAS.status_init)
@kopf.on.create(group, version, RESOURCES.dfts, when=LAMBDAS.status_init)
def dft_opr_on_dft_init(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    param.workflow_func = dft_create
    run_workflow(wffactory().CommonCreate(param=param))


@kopf.on.resume(group, version, RESOURCES.dfts, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.update(group, version, RESOURCES.dfts, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.create(group, version, RESOURCES.dfts, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
def dft_opr_on_dft_provisioned(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    param.workflow_func = dft_provisioned
    run_workflow(wffactory().CommonProvisioned(param=param))


@kopf.on.delete(group, version, RESOURCES.dfts, retries=OBJ_DEFAULTS.kopf_max_retries)
def dft_opr_on_dft_delete(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    param.workflow_func = dft_delete
    run_workflow(wffactory().CommonDelete(param=param))
