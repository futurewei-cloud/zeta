/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
* @file      trn_rpc_protocol.x
* @author    Sherif Abdelwahab,  <@zasherif>
*
* @brief Defines an internal protocol to manage the data-plane.
*
* @copyright Copyright (c) 2019 The Authors.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 2 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
*/

%// SPDX-License-Identifier: GPL-2.0-or-later
%#pragma GCC diagnostic ignored "-Wunused-variable"
%#include <stdint.h>

/*----- Data types. ----- */

/* At most 10 chains, size has to be prime and 100x number of chains */
const TRAN_MAX_MAGLEV_TABLE_SIZE = 10000;

/* Upper limit on maximum number of enpoint hosts */
const RPC_TRN_MAX_REMOTE_IPS = 256;

/* Defines generic codes, 0 is always a success need not to mention! */
const RPC_TRN_WARN = 1;
const RPC_TRN_ERROR = 2;
const RPC_TRN_FATAL = 3;
const RPC_TRN_NOT_IMPLEMENTED = 4;

struct rpc_trn_dft_t {
       string interface<20>;
       uint32_t id;
       uint32_t table<TRAN_MAX_MAGLEV_TABLE_SIZE>;
};

struct rpc_trn_ftn_t {
       string interface<20>;
       uint8_t position;
       uint32_t id;
       uint32_t ip;
       unsigned char mac[6];
       uint32_t next_ip;
       unsigned char next_mac[6];
};

struct rpc_trn_chain_t {
       string interface<20>;
       uint32_t id;
       uint32_t tail_ftn;
};

/* Defines an endpoint (all types) */
struct rpc_trn_endpoint_t {
       string interface<20>;
       uint32_t ip;
       uint32_t eptype;
       uint32_t remote_ips<RPC_TRN_MAX_REMOTE_IPS>;
       unsigned char mac[6];
       string hosted_interface<20>;
       string veth<20>;
       uint64_t tunid;
};

struct rpc_trn_zeta_key_t {
       string interface<20>;
       uint32_t id;
};

/* Defines a unique key to get/delete an RP (in DP) */
struct rpc_trn_endpoint_key_t {
       string interface<20>;
       uint64_t tunid;
       uint32_t ip;
};

/* Defines an interface and a path for xdp prog to load on the interface */
struct rpc_trn_xdp_intf_t {
       string interface<20>;
       string xdp_path<256>;
       string pcapfile<256>;
};

/* Defines an interface */
struct rpc_intf_t {
       string interface<20>;
};

/* Defines a tunneling interface (physical) */
struct rpc_trn_tun_intf_t {
       string interface<20>;
       uint32_t ip;
       unsigned char mac[6];
};


enum rpc_trn_pipeline_stage {
       ON_XDP_TX       = 0,
       ON_XDP_PASS     = 1,
       ON_XDP_REDIRECT = 2,
       ON_XDP_DROP     = 3,
       ON_XDP_SCALED_EP = 4
       /* add stages */
};

/* Defines an XDP program at xdp_path to be loaded at index prog_index */
struct rpc_trn_ebpf_prog_t {
       string interface<20>;
       rpc_trn_pipeline_stage stage;
       string xdp_path<256>;
};

/* Defines an XDP program at stage */
struct rpc_trn_ebpf_prog_stage_t {
       string interface<20>;
       rpc_trn_pipeline_stage stage;
};

/*----- Protocol. -----*/

program RPC_TRANSIT_REMOTE_PROTOCOL {
        version RPC_TRANSIT_ALFAZERO {
                int UPDATE_DFT(rpc_trn_dft_t) = 1;
                int DELETE_DFT(rpc_trn_zeta_key_t) = 2;
                rpc_trn_dft_t GET_DFT(rpc_trn_zeta_key_t) = 3;

                int UPDATE_FTN(rpc_trn_ftn_t) = 4;
                int DELETE_FTN(rpc_trn_zeta_key_t) = 5;
                rpc_trn_ftn_t GET_FTN(rpc_trn_zeta_key_t) = 6;

                int UPDATE_EP(rpc_trn_endpoint_t) = 7;
                int DELETE_EP(rpc_trn_endpoint_key_t) = 8;
                rpc_trn_endpoint_t GET_EP(rpc_trn_endpoint_key_t) = 9;

                int LOAD_TRANSIT_XDP(rpc_trn_xdp_intf_t) = 10;
                int UNLOAD_TRANSIT_XDP(rpc_intf_t) = 11;

                int LOAD_TRANSIT_XDP_PIPELINE_STAGE(rpc_trn_ebpf_prog_t) = 12;
                int UNLOAD_TRANSIT_XDP_PIPELINE_STAGE(rpc_trn_ebpf_prog_stage_t) = 13;

                int UPDATE_CHAIN(rpc_trn_chain_t) = 14;
                int DELETE_CHAIN(rpc_trn_zeta_key_t) = 15;
                rpc_trn_chain_t GET_CHAIN(rpc_trn_zeta_key_t) = 16;
          } = 1;

} =  0x20009051;
