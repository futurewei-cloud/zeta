# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import os
import subprocess

logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"))
logger = logging.getLogger()


class CONSTANTS:
    ACA_NODE = "ACA"


def run_cmd(cmd_list):
    result = subprocess.Popen(cmd_list, shell=True, stdout=subprocess.PIPE)
    text = result.stdout.read().decode().rstrip()
    return text
