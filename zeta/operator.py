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
import time
from kopf import cli
from zeta.common.wf_param import *
from zeta.dp.zeta.workflows.vpcs.triggers import *
from zeta.dp.zeta.workflows.nets.triggers import *
from zeta.dp.zeta.workflows.dividers.triggers import *
from zeta.dp.zeta.workflows.bouncers.triggers import *
from zeta.dp.zeta.workflows.droplets.triggers import *
from zeta.dp.zeta.workflows.endpoints.triggers import *
from zeta.dp.zeta.workflows.builtins.services.triggers import *
from zeta.dp.zeta.workflows.builtins.nodes.triggers import *
from zeta.dp.zeta.workflows.builtins.pods.triggers import *
import grpc
import threading
from google.protobuf import empty_pb2
from concurrent import futures
from zeta.proto import builtins_pb2_grpc as builtins_pb2_grpc
from zeta.arktos.arktos_service import ArktosService
from kubernetes import client, config
import socket
from zeta.common.constants import *

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

    start_time = time.time()

    run_workflow(wffactory().DropletOperatorStart(param=param))

    logger.info("Bootstrap time:  %s seconds ---" % (time.time() - start_time))
