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

import logging
import json
from common.common import run_cmd
from common.constants import KIND

logger = logging.getLogger()


class TrnRpc:
    def __init__(self, ip, benchmark=False):
        self.ip = ip            # RPC server IP
        self.benchmark = benchmark

        # transitd cli commands
        self.trn_cli = f'''/trn_bin/transit -s {self.ip} '''
        self.trn_cli_load_transit_xdp = f'''{self.trn_cli} load-transit-xdp -j'''
        self.trn_cli_unload_transit_xdp = f'''{self.trn_cli} unload-transit-xdp -j'''
        self.trn_cli_update_droplet = f'''{self.trn_cli} update-droplet -j'''
        self.trn_cli_update_dft = f'''{self.trn_cli} update-dft -j'''
        self.trn_cli_update_chain = f'''{self.trn_cli} update-chain -j'''
        self.trn_cli_update_ftn = f'''{self.trn_cli} update-ftn -j'''
        self.trn_cli_update_ep = f'''{self.trn_cli} update-ep -j'''
        self.trn_cli_get_dft = f'''{self.trn_cli} get-dft -j'''
        self.trn_cli_get_chain = f'''{self.trn_cli} get-chain -j'''
        self.trn_cli_get_ftn = f'''{self.trn_cli} get-ftn -j'''
        self.trn_cli_get_ep = f'''{self.trn_cli} get-ep -j'''
        self.trn_cli_delete_dft = f'''{self.trn_cli} delete-dft -j'''
        self.trn_cli_delete_chain = f'''{self.trn_cli} delete-chain -j'''
        self.trn_cli_delete_ftn = f'''{self.trn_cli} delete-ep -j'''
        self.trn_cli_delete_ep = f'''{self.trn_cli} delete-ep -j'''
        self.trn_cli_load_ebpf_prog = f'''{self.trn_cli} load-ebpf-prog -j'''
        self.trn_cli_unload_ebpf_prog = f'''{self.trn_cli} unload-ebpf-prog -j'''

        if benchmark:
            self.xdp_path = "/trn_xdp/trn_transit_xdp_ebpf.o"
        else:
            self.xdp_path = "/trn_xdp/trn_transit_xdp_ebpf_debug.o"

    def load_transit_xdp(self, itf_tenant, itf_zgc, ibo_port):
        jsonconf = {
            "itf_tenant": itf_tenant,
            "itf_zgc": itf_zgc,
            "ibo_port": ibo_port,
        }
        if self.benchmark:
            jsonconf["debug_mode"] = 0
        else:
            jsonconf["debug_mode"] = 1

        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_load_transit_xdp} \'{jsonconf}\' '''
        logger.info("load_transit_xdp: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def unload_transit_xdp(self, itf_tenant, itf_zgc):
        jsonconf = {
            "itf_tenant": itf_tenant,
            "itf_zgc": itf_zgc,
            "ibo_port": 0,
        }
        if self.benchmark:
            jsonconf["debug_mode"] = 0
        else:
            jsonconf["debug_mode"] = 1
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_unload_transit_xdp} {jsonconf} '''
        logger.info("unload_transit_xdp: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def update_droplet(self, itf_conf):
        jsonconf = json.dumps(itf_conf)
        cmd = f'''{self.trn_cli_update_droplet} \'{jsonconf}\''''
        logger.info("update_droplet: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def update_dft(self, dft):
        jsonconf = {
            "id": dft.id,
            "table": dft.table
        }

        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_dft} \'{jsonconf}\''''
        logger.info("update_dft: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def update_chain(self, chain, ftn):
        jsonconf = {
            "id": chain.id,
            "tail_ftn": ftn.id
        }

        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_chain} \'{jsonconf}\''''
        logger.info("update_chain: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def update_ftn(self, ftn_id, ftn_droplet_obj, ftn_next_droplet_obj, position):
        ip = ftn_droplet_obj.ip
        mac = ftn_droplet_obj.mac
        next_ip = ""
        next_mac = ""
        if ftn_next_droplet_obj:
            next_ip = ftn_next_droplet_obj.ip
            next_mac = ftn_next_droplet_obj.mac
        jsonconf = {
            "id": ftn_id,
            "ftn_position": position,
            "ip": ip,
            "mac": mac,
            "next_ip": next_ip,
            "next_mac": next_mac
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_update_ftn} \'{jsonconf}\''''
        logger.info("update_ftn: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def get_dft(self, dft):
        jsonconf = {
            "id": dft.id,
        }

        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_get_dft} \'{jsonconf}\''''
        logger.info("get_dft: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def get_chain(self, chain):
        jsonconf = {
            "id": chain.id,
        }

        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_get_chain} \'{jsonconf}\''''
        logger.info("get_chain: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def get_ftn(self, ftn):
        jsonconf = {
            "id": ftn.id,
        }

        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_get_ftn} \'{jsonconf}\''''
        logger.info("get_dft: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def delete_dft(self, dft):
        jsonconf = {
            "id": dft.id,
        }

        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_dft} \'{jsonconf}\''''
        logger.info("delete_dft: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def delete_chain(self, chain):
        jsonconf = {
            "id": chain.id,
        }

        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_chain} \'{jsonconf}\''''
        logger.info("delete_dft: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def delete_ftn(self, ftn_id):
        jsonconf = {
            "id": ftn_id,
        }

        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_ftn} \'{jsonconf}\''''
        logger.info("delete_dft: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def get_substrate_ep_json(self, ip, mac):
        jsonconf = {
            "tunnel_id": "0",
            "ip": ip,
            "eptype": "0",
            "mac": mac,
            "veth": "",
            "remote_ips": [""],
            "hosted_iface": ""
        }
        jsonconf = json.dumps(jsonconf)
        return jsonconf

    def update_substrate_ep(self, ip, mac):
        jsonconf = self.get_substrate_ep_json(ip, mac)
        cmd = f'''{self.trn_cli_update_ep} \'{jsonconf}\''''
        logger.info("update_substrate_ep: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def update_ep(self, ep_conf):
        jsonconf = json.dumps(ep_conf)
        cmd = f'''{self.trn_cli_update_ep} \'{jsonconf}\''''
        logger.info("update_ep: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))

    def delete_substrate_ep(self, ip):
        jsonconf = {
            "tunnel_id": "0",
            "ip": ip,
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_ep} \'{jsonconf}\''''
        logger.info("delete_substrate_ep: {}".format(cmd))
        returncode, text = run_cmd(cmd)
        logger.info(
            "delete_substrate_ep returns {} {}".format(returncode, text))

    def delete_ep(self, ep):
        jsonconf = {
            "tunnel_id": ep.get_tunnel_id(),
            "ip": ep.get_ip(),
        }
        jsonconf = json.dumps(jsonconf)
        cmd = f'''{self.trn_cli_delete_ep} \'{jsonconf}\''''
        log_string = "delete_ep for a {} {}".format(ep.type, ep.ip)
        logger.info(log_string)
        returncode, text = run_cmd(cmd)
        logger.info("returns {} {}".format(returncode, text))
