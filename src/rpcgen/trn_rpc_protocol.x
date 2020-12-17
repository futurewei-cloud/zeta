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
%#include "trn_datamodel.h"

/*----- Data types. ----- */

/* Defines generic codes, 0 is always a success need not to mention! */
const RPC_TRN_WARN = 1;
const RPC_TRN_ERROR = 2;
const RPC_TRN_FATAL = 3;
const RPC_TRN_NOT_IMPLEMENTED = 4;

typedef string rpc_intf_name<TRAN_MAX_ITF_SIZE>; 

struct rpc_trn_dft_t {
       uint32_t id;
       uint32_t table<TRAN_MAX_MAGLEV_TABLE_SIZE>;
};

struct rpc_trn_ftn_t {
       uint8_t position;
       uint32_t id;
       uint32_t ip;
       unsigned char mac[6];
       uint32_t next_ip; /* 0 if tail */
       unsigned char next_mac[6]; /* empty string if tail */
};

struct rpc_trn_chain_t {
       uint32_t id;
       uint32_t tail_ftn;
};

struct rpc_trn_zeta_key_t {
       uint32_t id;
};

struct rpc_addr_t {
	uint32_t ip;
	uint8_t mac[6];
};

/* Defines an endpoint */
struct rpc_endpoint_key_t {
       uint32_t vni;
	uint32_t ip;
};

/*
struct rpc_endpoint_t {
	uint32_t hip;
	uint8_t mac[6];
	uint8_t hmac[6];
};
*/
struct rpc_trn_endpoint_t {
       uint64_t buf64[3];
};

/* endpoints batch, watch for 8k buffer limit over UDP */
typedef struct rpc_trn_endpoint_t rpc_trn_endpoint_batch_t<TRAN_MAX_EP_BATCH_SIZE>;

/* Defines a droplet (physical interface) */
struct rpc_trn_droplet_t {
       rpc_intf_name interface;
       uint32_t num_entrances;
       rpc_addr_t entrances[TRAN_MAX_ZGC_ENTRANCES];
};

/* Defines interfaces for xdp prog to attach/detatch */
struct rpc_trn_xdp_intf_t {
       rpc_intf_name interfaces[TRAN_ITF_MAP_MAX];
       uint16_t ibo_port;
       uint32_t debug_mode;
};

/* Defines an ebpf program at path to be loaded */
struct rpc_trn_ebpf_prog_t {
       uint32_t prog_idx;
       uint32_t debug_mode;
};

/*----- Protocol. -----*/

program RPC_TRANSIT_REMOTE_PROTOCOL {
        version RPC_TRANSIT_ALFAZERO {
                int LOAD_TRANSIT_XDP(rpc_trn_xdp_intf_t) = 1;
                int UNLOAD_TRANSIT_XDP(rpc_trn_xdp_intf_t) = 2;

                int LOAD_TRANSIT_XDP_EBPF(rpc_trn_ebpf_prog_t) = 3;
                int UNLOAD_TRANSIT_XDP_EBPF(rpc_trn_ebpf_prog_t) = 4;

                int UPDATE_EP(rpc_trn_endpoint_batch_t) = 5;
                int DELETE_EP(rpc_endpoint_key_t) = 6;
                rpc_trn_endpoint_t GET_EP(rpc_endpoint_key_t) = 7;

                int UPDATE_DFT(rpc_trn_dft_t) = 8;
                int DELETE_DFT(rpc_trn_zeta_key_t) = 9;
                rpc_trn_dft_t GET_DFT(rpc_trn_zeta_key_t) = 10;

                int UPDATE_FTN(rpc_trn_ftn_t) = 11;
                int DELETE_FTN(rpc_trn_zeta_key_t) = 12;
                rpc_trn_ftn_t GET_FTN(rpc_trn_zeta_key_t) = 13;

                int UPDATE_CHAIN(rpc_trn_chain_t) = 14;
                int DELETE_CHAIN(rpc_trn_zeta_key_t) = 15;
                rpc_trn_chain_t GET_CHAIN(rpc_trn_zeta_key_t) = 16;

                int UPDATE_DROPLET(rpc_trn_droplet_t) = 17;
          } = 1;

} =  0x20009051;
