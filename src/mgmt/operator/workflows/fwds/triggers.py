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
from workflows.fwds.create import *
from workflows.fwds.delete import *
from workflows.fwds.provisioned import *


@kopf.on.resume(group, version, RESOURCES.fwds, when=LAMBDAS.status_init)
@kopf.on.update(group, version, RESOURCES.fwds, when=LAMBDAS.status_init)
@kopf.on.create(group, version, RESOURCES.fwds, when=LAMBDAS.status_init)
def fwd_opr_on_fwd_init(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    param.workflow_func = fwd_create
    run_workflow(wffactory().CommonCreate(param=param))


@kopf.on.resume(group, version, RESOURCES.fwds, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.update(group, version, RESOURCES.fwds, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.create(group, version, RESOURCES.fwds, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
def fwd_opr_on_fwd_provisioned(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    param.workflow_func = fwd_provisioned
    run_workflow(wffactory().CommonProvisioned(param=param))


@kopf.on.delete(group, version, RESOURCES.fwds, retries=OBJ_DEFAULTS.kopf_max_retries)
def fwd_opr_on_fwd_delete(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    param.workflow_func = fwd_delete
    run_workflow(wffactory().CommonDelete(param=param))
