# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
from kubernetes.client.rest import ApiException
from kubernetes import client, config
import logging
logger = logging.getLogger()


config.load_incluster_config()
core_api = client.CoreV1Api()


def init():
    global activeZgc
    activeZgc = {}
    global node_ips
    node_ips = {}
    global vnis
    vnis = {}
    global zgc_cidr_range
    # Give it a default value.
    zgc_cidr_range = '10.0.0.0/16'
    global MAC_PRIVATE
    # Give it a default
    MAC_PRIVATE = 0x928300000000
    try:
        api_response = core_api.read_namespaced_config_map(
            name="zgc-cluster-config",
            namespace="default"
        )
        logger.info('Result for getting configmap: {}'.format(api_response))
        zgc_cidr_range = api_response.data['cidr']
        MAC_PRIVATE = int(api_response.data['MAC_PRIVATE'], 0)
    except ApiException as e:
        logger.error(
            'Exception when reading configmap {} : {}'.format("zgc-cluster-config", e))
