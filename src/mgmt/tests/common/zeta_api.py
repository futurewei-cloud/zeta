import logging
import requests
import json
from src.mgmt.tests.common.k8s_helper import K8sHelper

logger = logging.getLogger()


class ZetaApi:

    def __init__(self):
        manager_ip_cmd = "kubectl get node $(kubectl get pods -o wide | grep zeta-manager | awk '{print $7}' | cut -d/ -f1) -o wide | grep kind | awk '{print $6}' | cut -d/ -f1"
        self.ip = K8sHelper.run_cmd(manager_ip_cmd)
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
        return self.zeta_http_request_json("post", "vpcs", data)

    def create_port(self, data):
        return self.zeta_http_request_json("post", "ports", data)

    def delete_vpc(self, vpc_id):
        return self.zeta_http_request_json("delete", "vpcs/{}".format(vpc_id), {})

    def delete_port(self, port_id):
        return self.zeta_http_request_json("delete", "ports/{}".format(port_id), {})

    def create_node(self):
        pass
