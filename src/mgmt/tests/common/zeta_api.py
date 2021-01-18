# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import requests
import json
from src.mgmt.tests.common.common import run_cmd, logger


class ZetaApi:

    def __init__(self):
        manager_ip_cmd = "kubectl get node $(kubectl get pods -o wide | grep zeta-manager | awk '{print $7}' | cut -d/ -f1) -o wide | grep kind | awk '{print $6}' | cut -d/ -f1"
        self.ip = run_cmd(manager_ip_cmd)
        self.port = "80"
        self.address = "http://{}:{}".format(self.ip, self.port)

    def zeta_http_request_json(self, verb, resource, data):
        headers = {'Content-type': 'application/json'}
        if verb == "post":
            response = requests.post("{}/{}".format(self.address, resource),
                                     data=data, headers=headers)
        elif verb == "get":
            response = requests.get("{}/{}".format(self.address, resource),
                                    data=data, headers=headers)
        elif verb == "put":
            response = requests.put("{}/{}".format(self.address, resource),
                                    data=data, headers=headers)
        elif verb == "patch":
            response = requests.patch("{}/{}".format(self.address, resource),
                                      data=data, headers=headers)
        elif verb == "delete":
            response = requests.delete(
                "{}/{}".format(self.address, resource), headers=headers)
        return response

    def create_vpc(self, data):
        return self.zeta_http_request_json("post", "vpcs", json.dumps(data))

    def create_port(self, data):
        return self.zeta_http_request_json("post", "ports", json.dumps(data))

    def create_node(self):
        pass

    def create_zgc(self):
        pass

    def delete_vpc(self, vpc_id):
        pass

    def delete_port(self, port_id):
        pass
