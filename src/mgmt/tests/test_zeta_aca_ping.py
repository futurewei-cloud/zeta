# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import unittest
import os
import json
from time import sleep
from src.mgmt.tests.common.node import Node
from src.mgmt.tests.common.zeta_api import ZetaApi
from src.mgmt.tests.common.aca_helper import AcaHelper
from src.mgmt.tests.common.common import logger, CONSTANTS
from src.mgmt.tests.common.test_helper import do_ping_test


class test_zeta_aca_ping(unittest.TestCase):

    def setUp(self):
        self.zeta_api = ZetaApi()
        self.aca = AcaHelper("src/extern/alcor-control-agent")
        self.node_1 = Node("aca_node_1", "tenant_network", CONSTANTS.ACA_NODE)
        self.node_2 = Node("aca_node_2", "tenant_network", CONSTANTS.ACA_NODE)
        self.vpc_id = "3dda2801-d675-4688-a63f-dcda8d327f61"
        self.vni = "888"
        self.node_1.ports.append(
            {
                "id": "333d4fae-7dec-11d0-a765-00a0c9341120",
                "ip": "10.10.0.92",
                "mac": "cc:dd:ee:ff:11:22"
            }
        )
        self.node_2.ports.append(
            {
                "id": "99976feae-7dec-11d0-a765-00a0c9342230",
                "ip": "10.10.0.93",
                "mac": "6c:dd:ee:ff:11:32"
            }
        )
        vpc = {
            "vpc_id": self.vpc_id,
            "vni": self.vni
        }
        ports = []
        for port in self.aca.get_ports_aca_format(self.node_1, self.vpc_id):
            ports.append(port)
        for port in self.aca.get_ports_aca_format(self.node_2, self.vpc_id):
            ports.append(port)

        json_content_for_aca = {}
        json_content_for_aca['vpc_response'] = self.vpc_response = self.zeta_api.create_vpc(
            vpc).json()
        json_content_for_aca['port_response'] = self.ports_response = self.zeta_api.create_port(
            ports).json()
        self.aca.write_test_data_file(json_content_for_aca)

        # Static ARP entries
        for gws in json_content_for_aca["vpc_response"]["gws"]:
            self.node_1.run("arp -s {} {}".format(gws["ip"], gws["mac"]))
            self.node_2.run("arp -s {} {}".format(gws["ip"], gws["mac"]))

        testcases_to_run = ['DISABLED_zeta_scale_CHILD',
                            'DISABLED_zeta_scale_PARENT']
        self.aca.run_aca_test_case(testcases_to_run[0], self.node_1)
        self.aca.run_aca_test_case(testcases_to_run[1], self.node_2)

    def tearDown(self):
        pass

    def test_zeta_aca_ping(self):
        do_ping_test(
            self, self.node_1, self.node_1.ports[0]["ip"], self.node_2, self.node_2.ports[0]["ip"])
