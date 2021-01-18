# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import json

class AcaHelper:

    def __init__(self, aca_path):
        self.path = aca_path

    def get_ports_aca_format(self, node, vpc_id):
        ports_list = []
        for port in node.ports:
            ports_list.append(
                {
                    "port_id": port["id"],
                    "vpc_id": vpc_id,
                    "ips_port": [{
                        "ip": port["ip"],
                        "vip": ""
                    }],
                    "mac_port": port["mac"],
                    "ip_node": node.ip,
                    "mac_node": node.mac
                }
            )
        return ports_list

    def write_test_data_file(self, data):
        with open(f"{self.path}/test/gtest/aca_data.json", 'w') as outfile:
            json.dump(data, outfile)

    def run_aca_test_case(self, test_case, node):
        cmd = (f'''bash -c '\
cd /mnt/Zeta/{self.path} && ./build/tests/aca_tests --gtest_also_run_disabled_tests --gtest_filter=*{test_case}' ''')
        node.run(cmd)
