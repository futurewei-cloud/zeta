from src.mgmt.tests.common.node import Node
from src.mgmt.tests.common.zeta_api import ZetaApi
import unittest
import logging
import os
import json
from time import sleep

logger = logging.getLogger()


class test_zeta_aca_simple(unittest.TestCase):

    def setUp(self):
        logger.info("")
        self.test_name = "test_zeta_aca"
        self.zeta_api = ZetaApi()
        self.node_1 = Node("aca_node_1", "tenant_network")
        self.node_2 = Node("aca_node_2", "tenant_network")
        self.node_1.ports.append("10.10.0.92")
        self.node_2.ports.append("10.10.0.93")
        logger.info(type(self.node_1.ip))
        logger.info(type(self.node_1.mac))
        vpc = {
            "vpc_id": "3dda2801-d675-4688-a63f-dcda8d327f61",
            "vni": "888"
        }
        ports = [{
            "port_id": "333d4fae-7dec-11d0-a765-00a0c9341120",
            "vpc_id": "3dda2801-d675-4688-a63f-dcda8d327f61",
            "ips_port": [{
                "ip": "10.10.0.92",
                "vip": ""
            }],
            "mac_port": "cc:dd:ee:ff:11:22",
            "ip_node": self.node_1.ip,
            "mac_node": self.node_1.mac
        },
            {
            "port_id": "99976feae-7dec-11d0-a765-00a0c9342230",
                "vpc_id": "3dda2801-d675-4688-a63f-dcda8d327f61",
                "ips_port": [{
                    "ip": "10.10.0.93",
                    "vip": ""
                }],
            "mac_port": "6c:dd:ee:ff:11:32",
            "ip_node": self.node_2.ip,
            "mac_node": self.node_2.mac
        }
        ]
        vpc = json.dumps(vpc)
        ports = json.dumps(ports)
        json_content_for_aca = dict()
        json_content_for_aca['vpc_response'] = self.vpc_response = self.zeta_api.create_vpc(
            vpc).json()
        json_content_for_aca['port_response'] = self.ports_response = self.zeta_api.create_port(
            ports).json()
        with open('alcor-control-agent/test/gtest/aca_data.json', 'w') as outfile:
            json.dump(json_content_for_aca, outfile)

        for gws in json_content_for_aca["vpc_response"]["gws"]:
            self.node_1.run("arp -s {} {}".format(gws["ip"], gws["mac"]))
            self.node_2.run("arp -s {} {}".format(gws["ip"], gws["mac"]))

        testcases_to_run = ['DISABLED_zeta_scale_CHILD',
                            'DISABLED_zeta_scale_PARENT']

        cmd_child = (f''' sudo bash -c '\
cd /mnt/Zeta/alcor-control-agent && ./build/tests/aca_tests --gtest_also_run_disabled_tests --gtest_filter=*{testcases_to_run[0]}' ''')
        cmd_parent = (f''' sudo bash -c '\
cd /mnt/Zeta/alcor-control-agent && ./build/tests/aca_tests --gtest_also_run_disabled_tests --gtest_filter=*{testcases_to_run[1]}' ''')
        self.node_1.run(cmd_child)
        self.node_2.run(cmd_parent)

    def tearDown(self):
        pass
        # vpc = "3dda2801-d675-4688-a63f-dcda8d327f61"
        # logger.info(self.zeta_api.delete_vpc(vpc).json())
        # port = "333d4fae-7dec-11d0-a765-00a0c9341120"
        # logger.info(self.zeta_api.delete_port(port).json())
        # port = "99976feae-7dec-11d0-a765-00a0c9342230"
        # logger.info(self.zeta_api.delete_port(port).json())

    def test_zeta_aca_simple(self):
        # execute ping
        self.node_1.run(
            "ping -I {} {} -c1".format(self.node_1.ports[0], self.node_2.ports[0]))
        input()
