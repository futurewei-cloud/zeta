// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_xdp_usr.h
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief User space APIs and data structures to program transit
 * xdp program.
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

#include <argp.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <linux/bpf.h>
#include <linux/if_link.h>
#include <net/if.h>
#include <netinet/in.h>
#include <search.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>

#include "bpf/bpf.h"
#include "bpf/libbpf.h"

#include "extern/cJSON.h"

typedef struct {
	int prog_id;          // definition in trn_xdp_prog_id_t
	char *prog_path;      // full path of xdp program
} trn_xdp_prog_t;

typedef struct {
	char *name;
	bool shared;
	int fd;
	struct bpf_map *map;
} trn_xdp_map_t;

typedef struct {
	int prog_fd;
	__u32 prog_id;
	struct bpf_object *obj;
	char pcapfile[TRAN_MAX_PATH_SIZE];
} trn_prog_t;

typedef struct {
	__u32 ip;
	__u32 iface_index;
	__u16 ibo_port;
	__u8 protocol;     // value from trn_xdp_tunnel_protocol_t
	__u8 role;         // value from trn_xdp_role_t
	__u8 mac[6];       // MAC of physical interface
} trn_iface_t;

typedef struct {
	trn_iface_t eth;
	trn_prog_t xdp;
} trn_xdp_object_t;

typedef struct {
	bool ready;
	__u32 xdp_flags;
	trn_xdp_object_t objs[TRAN_ITF_MAP_MAX];

	trn_xdp_prog_t *prog_tbl;

	/*
	 * Array of sidecar programs transit XDP main program can jump to
	 * to perform additional functionality.
	 */
	trn_prog_t ebpf_progs[TRAN_MAX_PROG];
} user_metadata_t;

trn_iface_t *trn_get_itf_context(char *interface);

int trn_update_itf_config(struct tunnel_iface_t *itf);

int trn_update_dft(__u32 *dft_key, struct dft_t *dft);

int trn_update_chain(__u32 *ftn_key, struct chain_t *chain);

int trn_update_ftn(__u32 *ftn_key, struct ftn_t *ftn);

int trn_update_endpoints_get_ctx(void);

int trn_update_endpoint(int fd, endpoint_key_t *epkey, endpoint_t *ep);

int trn_get_dft(__u32 *dft_key, struct dft_t *dft);

int trn_get_chain(__u32 *chain_key, struct chain_t *chain);

int trn_get_ftn(__u32 *ftn_key, struct ftn_t *ftn);

int trn_get_endpoint(endpoint_key_t *epkey, endpoint_t *ep);

int trn_delete_dft(__u32 *zeta_key);

int trn_delete_chain(__u32 *zeta_key);

int trn_delete_ftn(__u32 *zeta_key);

int trn_delete_endpoint(endpoint_key_t *epkey);

int trn_transit_xdp_load(char **interfaces, unsigned short ibo_port, bool debug);

int trn_transit_xdp_unload(char **interfaces);

int trn_transit_ebpf_load(int prog_idx);

int trn_transit_ebpf_unload(int prog_idx);

int trn_transit_dp_assistant(void);
