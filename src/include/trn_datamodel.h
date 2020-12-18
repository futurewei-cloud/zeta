// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file datamodel.h
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief Data models between user and kernel space. data propagated
 * from control-plane through transitd.
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
#pragma once

#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/types.h>
#define __ALIGNED_64__ __attribute__((aligned(64)))
#define __ALWAYS_INLINE__ __attribute__((__always_inline__))

/* maximal string size, including null terminator */
#define TRAN_MAX_PATH_SIZE 256
#define TRAN_MAX_ITF_SIZE 20
#define TRAN_MAX_CLI_JSON_STR 10240

#define TRAN_MAX_ZGC_ENTRANCES 128
#define TRAN_MAX_EP_BATCH_SIZE 360
#define TRAN_DP_FLOW_TIMEOUT 30     // In seconds

/* At most 10 chains, size has to be prime and 100x number of chains */
#define TRAN_MAX_MAGLEV_TABLE_SIZE 10000
#define TRAN_MAX_FTN 512
#define TRAN_MAX_CHAIN 128

/* Set max total number of endpoints */
#define TRAN_MAX_NEP 128*1024
/* Set max number of host IPs a (scaled) endpoint can be mapped to */
#define TRAN_MAX_REMOTES 64
#define TRAN_MAX_ITF 128
#define TRAN_MAX_VETH 2048
#define TRAN_UNUSED_ITF_IDX -1

#define TRAN_SUBSTRT_VNI 0

#define TRAN_SUBSTRT_EP 0
#define TRAN_SIMPLE_EP 1
#define TRAN_SCALED_EP 2
#define TRAN_GATEWAY_EP 3

/* Size for OAM message queue bpfmap */
#define TRAN_OAM_QUEUE_LEN 1024

/* XDP interface_map keys for packet redirect */
enum trn_itf_ma_key_t {
TRAN_ITF_MAP_TENANT = 0,     // id map to ifindex connected to tenant network
TRAN_ITF_MAP_ZGC,            // id map to ifindex connected to zgc network
TRAN_ITF_MAP_MAX
};

/* Cache related const */
#define TRAN_MAX_CACHE_SIZE 1000000

/* XDP programs keys in transit XDP tailcall jump table */
enum trn_xdp_prog_id_t {
	TRAN_TRANSIT_PROG = 0,
	TRAN_TX_PROG,
	TRAN_PASS_PROG,
	TRAN_REDIRECT_PROG,
	TRAN_DROP_PROG,
	TRAN_MAX_PROG
};

/* XDP programs roles pass along tail-called bpf programs */
enum trn_xdp_role_t {
	XDP_FWD = 0,
	XDP_FTN,
	XDP_ROLE_MAX
};

/* FTN node position/type in DFT chain */
enum trn_ftn_type_t {
    TRAN_FTN_TYPE_HEAD = 0,
    TRAN_FTN_TYPE_MIDDLE,
    TRAN_FTN_TYPE_TAIL
};

/* Tunnel Interface protocol */
enum trn_xdp_tunnel_protocol_t {
	XDP_TUNNEL_VXLAN = 0,
	XDP_TUNNEL_GENEVE
};

/* Flow Verdict */
enum trn_xdp_flow_op_t {
	XDP_FLOW_OP_ENCAP = 0,
	XDP_FLOW_OP_DELETE,
	XDP_FLOW_OP_DROP,
	XDP_FLOW_OP_MAX
};

/* Tie interface definitions together */
typedef struct {
	int itf_map_key;    // from trn_itf_ma_key_t
	int itf_xdp_role;   // from trn_xdp_role_t
	int itf_protocol;   // from trn_xdp_tunnel_protocol_t
} trn_xdp_itf_def_t;

struct dft_t {
	__u32 table_len;
	__u32 table[TRAN_MAX_MAGLEV_TABLE_SIZE];
} __attribute__((packed, aligned(4)));

struct chain_t {
	__u32 tail_ftn;
	__u32 tail_ftn_ip;
	unsigned char tail_ftn_mac[6];
} __attribute__((packed, aligned(4)));

struct ftn_t {
	__u8 position;
	__u32 ip;
	__u32 next_ip;
	unsigned char mac[6];
	unsigned char next_mac[6];
} __attribute__((packed, aligned(4)));

typedef struct {
	__u32 vni;
	__u32 ip;
} __attribute__((packed, aligned(4))) endpoint_key_t;

typedef struct {
	__u32 hip;
	unsigned char mac[6];
	unsigned char hmac[6];
} __attribute__((packed, aligned(4))) endpoint_t;

typedef struct {
	__u32 ip;   // IP used for ZGC access
	__u16 announced;    // non-zero indicates the MAC has been announced locally
	__u8 mac[6]; // MAC to be used for ZGC entrance
} __attribute__((packed, aligned(4))) zgc_entrance_t;

/* Should call it overlay interface */
struct tunnel_iface_t {
	__u32 iface_index;
	__u16 ibo_port;
	__u8 protocol;     // value from trn_xdp_tunnel_protocol_t
	__u8 role;         // value from trn_xdp_role_t
	__u32 num_entrances;  // number of valid entries in entrances array
	zgc_entrance_t entrances[TRAN_MAX_ZGC_ENTRANCES];
} __attribute__((packed, aligned(4)));

typedef struct {
	__u32 saddr;
	__u32 daddr;
	__u16 sport;
	__u16 dport;
	__u8 protocol;
	__u8 vni[3];
} __attribute__((packed, aligned(4))) ipv4_flow_t;

struct remote_endpoint_t {
	__u32 ip;
	unsigned char mac[6];
} __attribute__((packed));

struct scaled_endpoint_remote_t {
	/* Addresses */
	__u32 saddr;
	__u32 daddr;

	/* ports */
	__u16 sport;
	__u16 dport;

	unsigned char h_source[6];
	unsigned char h_dest[6];
} __attribute__((packed));

/* Direct Path oam op data */
typedef struct {
	/* Destination Encap */
	__u32 dip;
	__u32 dhip;
	__u8 dmac[6];
	__u8 dhmac[6];
	__u16 timeout;      /* in seconds */
	__u16 rsvd;
} dp_encap_opdata_t;

typedef struct {
	__u16 len;
	struct ethhdr eth;
	struct iphdr ip;
	struct udphdr udp;
	__u32 opcode;       // trn_xdp_flow_op_t

	/* OAM OpData */
	ipv4_flow_t flow;	// flow matcher

	union {
		dp_encap_opdata_t encap;
	} opdata;
} __attribute__((packed, aligned(8))) flow_ctx_t;
