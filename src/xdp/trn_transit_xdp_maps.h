// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file transit_maps.h
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief Defines ebpf maps of transit XDP
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

#include <linux/bpf.h>

#include "extern/bpf_helpers.h"
#include "extern/xdpcap_hook.h"

#include "trn_datamodel.h"

#define MAX_EP 65537
#define MAX_FTN 512
#define MAX_CHAIN 128

struct bpf_map_def SEC("maps") jmp_table = {
	.type = BPF_MAP_TYPE_PROG_ARRAY,
	.key_size = sizeof(__u32),
	.value_size = sizeof(__u32),
	.max_entries = TRAN_MAX_PROG,
};
BPF_ANNOTATE_KV_PAIR(jmp_table, __u32, __u32);

struct bpf_map_def SEC("maps") dfts_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(__u32),
	.value_size = sizeof(struct dft_t),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(dfts_map, __u32, struct dft_t);

struct bpf_map_def SEC("maps") chains_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(__u32),
	.value_size = sizeof(struct chain_t),
	.max_entries = MAX_CHAIN,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(chains_map, __u32, struct chain_t);

struct bpf_map_def SEC("maps") ftns_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(__u32),
	.value_size = sizeof(struct ftn_t),
	.max_entries = MAX_FTN,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(ftns_map, __u32, struct ftn_t);

struct bpf_map_def SEC("maps") endpoints_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(struct endpoint_key_t),
	.value_size = sizeof(struct endpoint_t),
	.max_entries = MAX_EP,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(endpoints_map, struct endpoint_key_t, struct endpoint_t);

struct bpf_map_def SEC("maps") hosted_endpoints_iface_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(struct endpoint_key_t),
	.value_size = sizeof(int),
	.max_entries = MAX_EP,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(hosted_endpoints_iface_map, struct endpoint_key_t, int);

struct bpf_map_def SEC("maps") interface_config_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(int),
	.value_size = sizeof(struct tunnel_iface_t),
	.max_entries = 1,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(interface_config_map, int, struct tunnel_iface_t);

struct bpf_map_def SEC("maps") interfaces_map = {
	.type = BPF_MAP_TYPE_DEVMAP,
	.key_size = sizeof(int),
	.value_size = sizeof(int),
	.max_entries = TRAN_MAX_ITF,
};
BPF_ANNOTATE_KV_PAIR(interface_map, int, int);

struct bpf_map_def SEC("maps") fwd_flow_mod_cache = {
	.type = BPF_MAP_TYPE_LRU_HASH,
	.key_size = sizeof(struct ipv4_tuple_t),
	.value_size = sizeof(struct scaled_endpoint_remote_t),
	.max_entries = TRAN_MAX_CACHE_SIZE,
};
BPF_ANNOTATE_KV_PAIR(fwd_flow_mod_cache, struct ipv4_tuple_t,
		     struct scaled_endpoint_remote_t);

struct bpf_map_def SEC("maps") rev_flow_mod_cache = {
	.type = BPF_MAP_TYPE_LRU_HASH,
	.key_size = sizeof(struct ipv4_tuple_t),
	.value_size = sizeof(struct scaled_endpoint_remote_t),
	.max_entries = TRAN_MAX_CACHE_SIZE,
};
BPF_ANNOTATE_KV_PAIR(rev_flow_mod_cache, struct ipv4_tuple_t,
		     struct scaled_endpoint_remote_t);

struct bpf_map_def SEC("maps") ep_flow_host_cache = {
	.type = BPF_MAP_TYPE_LRU_HASH,
	.key_size = sizeof(struct ipv4_tuple_t),
	.value_size = sizeof(struct remote_endpoint_t),
	.max_entries = TRAN_MAX_CACHE_SIZE,
};
BPF_ANNOTATE_KV_PAIR(ep_flow_host_cache, struct ipv4_tuple_t,
		     struct remote_endpoint_t);

struct bpf_map_def SEC("maps") ep_host_cache = {
	.type = BPF_MAP_TYPE_LRU_HASH,
	.key_size = sizeof(struct endpoint_key_t),
	.value_size = sizeof(struct remote_endpoint_t),
	.max_entries = TRAN_MAX_CACHE_SIZE,
};
BPF_ANNOTATE_KV_PAIR(ep_host_cache, struct endpoint_key_t,
		     struct remote_endpoint_t);

struct bpf_map_def SEC("maps") xdpcap_hook = XDPCAP_HOOK();
