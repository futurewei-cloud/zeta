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


@kopf.on.resume(group, version, RESOURCES.ftns, when=LAMBDAS.status_init, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.update(group, version, RESOURCES.ftns, when=LAMBDAS.status_init, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.create(group, version, RESOURCES.ftns, when=LAMBDAS.status_init, retries=OBJ_DEFAULTS.kopf_max_retries)
def ftn_opr_on_ftn_init(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    run_workflow(wffactory().FtnCreate(param=param))


@kopf.on.resume(group, version, RESOURCES.ftns, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.update(group, version, RESOURCES.ftns, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
@kopf.on.create(group, version, RESOURCES.ftns, when=LAMBDAS.status_provisioned, retries=OBJ_DEFAULTS.kopf_max_retries)
def ftn_opr_on_ftn_provisioned(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    run_workflow(wffactory().FtnProvisioned(param=param))


@kopf.on.delete(group, version, RESOURCES.ftns, retries=OBJ_DEFAULTS.kopf_max_retries)
def ftn_opr_on_ftn_delete(body, spec, **kwargs):
    param = HandlerParam()
    param.name = kwargs['name']
    param.body = body
    param.spec = spec
    run_workflow(wffactory().FtnDelete(param=param))
