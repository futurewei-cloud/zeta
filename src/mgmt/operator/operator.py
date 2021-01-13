#!/usr/bin/python3

# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:The above copyright
# notice and this permission notice shall be included in all copies or
# substantial portions of the Software.THE SOFTWARE IS PROVIDED "AS IS",
# WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import subprocess
import logging
import time
import asyncio
import os.path
import grpc
import threading
import socket
import pathlib
from kopf import cli
from common.wf_param import *
from common.wf_factory import *
from kubernetes import client, config
from common.constants import *
from common.common import *
from workflows.droplets.triggers import *
from workflows.fwds.triggers import *
from workflows.ftns.triggers import *
from workflows.chains.triggers import *
from workflows.dfts.triggers import *
logger = logging.getLogger()
LOCK: asyncio.Lock
POOL_WORKERS = 10


@kopf.on.startup()
async def on_startup(logger, **kwargs):
    start_time = time.time()
    global LOCK
    LOCK = asyncio.Lock()
    param = HandlerParam()
    config.load_incluster_config()
    sched = 'luigid --background --port 8082 --pidfile /var/run/luigi/luigi.pid --logdir /var/log/luigi --state-path /var/lib/luigi/luigi.state'
    subprocess.call(sched, shell=True)
    while not os.path.exists("/var/run/luigi/luigi.pid"):
        pass
    with open('/var/run/luigi/luigi.pid', 'r') as f:
        pid = f.read()
    # Wait for daemon to run
    while not os.path.exists("/proc/{}".format(pid)):
        pass
    logger.info("Running luigid central scheduler pid={}!".format(pid))

    configmap = read_config_map()
    if configmap:
        OBJ_DEFAULTS.default_dft = configmap.data["default_dft"]
        OBJ_DEFAULTS.default_dft_id = configmap.data["default_dft_id"]
        OBJ_DEFAULTS.default_n_fwds = int(configmap.data["default_n_fwds"])
        OBJ_DEFAULTS.default_n_ftns = int(configmap.data["default_n_ftns"])
        OBJ_DEFAULTS.default_n_chains = int(configmap.data["default_n_chains"])

    start_time = time.time()

    run_workflow(wffactory().CommonOperatorStart(param=param))

    logger.info("Bootstrap time:  %s seconds ---" % (time.time() - start_time))


def read_config_map():
    return kube_read_config_map(client.CoreV1Api(), "zgc-cluster-config", "default")
