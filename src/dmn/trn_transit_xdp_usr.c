// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief User space APIs to program transit xdp program (switches and
 * routers)
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

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include "extern/linux/err.h"
#include "trn_transitd.h"

static trn_xdp_prog_t trn_prog_tbl[TRAN_MAX_PROG] = {
	{TRAN_TRANSIT_PROG, "/trn_xdp/trn_transit_xdp_ebpf.o"},
	{TRAN_TX_PROG, "/trn_xdp/trn_transit_tx_proc_xdp_ebpf.o"},
	{TRAN_PASS_PROG, "/trn_xdp/trn_transit_pass_proc_xdp_ebpf.o"},
	{TRAN_REDIRECT_PROG, "/trn_xdp/trn_transit_redirect_proc_xdp_ebpf.o"},
	{TRAN_DROP_PROG, "/trn_xdp/trn_transit_drop_proc_xdp_ebpf.o"},
};

static trn_xdp_prog_t trn_prog_dbg_tbl[TRAN_MAX_PROG] = {
	{TRAN_TRANSIT_PROG, "/trn_xdp/trn_transit_xdp_ebpf_debug.o"},
	{TRAN_TX_PROG, "/trn_xdp/trn_transit_tx_proc_xdp_ebpf_debug.o"},
	{TRAN_PASS_PROG, "/trn_xdp/trn_transit_pass_proc_xdp_ebpf_debug.o"},
	{TRAN_REDIRECT_PROG, "/trn_xdp/trn_transit_redirect_proc_xdp_ebpf_debug.o"},
	{TRAN_DROP_PROG, "/trn_xdp/trn_transit_drop_proc_xdp_ebpf_debug.o"},
};

static trn_xdp_itf_def_t trn_xdp_itf_def[TRAN_ITF_MAP_MAX] = {
	{TRAN_ITF_MAP_TENANT, XDP_FWD, XDP_TUNNEL_VXLAN},
	{TRAN_ITF_MAP_ZGC, XDP_FTN, XDP_TUNNEL_GENEVE},
};

static char * pin_path = "/sys/fs/bpf";

/* Make sure to keep in-sync with XDP programs, order doesn't matter */
static trn_xdp_map_t trn_xdp_bpfmaps[] = {
	{"jmp_table", true, -1, NULL},
	{"dfts_map", true, -1, NULL},
	{"chains_map", true, -1, NULL},
	{"ftns_map", true, -1, NULL},
	{"endpoints_map", true, -1, NULL},
	{"hosted_eps_if", true, -1, NULL},
	{"if_config_map", true, -1, NULL},
	{"interfaces_map", true, -1, NULL},
	{"oam_queue_map", true, -1, NULL},
	{"fwd_flow_cache", true, -1, NULL},
	{"rev_flow_cache", true, -1, NULL},
	{"host_flow_cache", true, -1, NULL},
	{"ep_host_cache", true, -1, NULL},
	{"xdpcap_hook", false, -1, NULL},
};

static user_metadata_t *md = NULL;

static trn_xdp_map_t * trn_transit_map_get(char *map_name)
{
	int num_maps = sizeof(trn_xdp_bpfmaps) / sizeof(trn_xdp_bpfmaps[0]);
	for (int i = 0; i < num_maps; i++) {
		if (strcmp(trn_xdp_bpfmaps[i].name, map_name) == 0) {
			return &trn_xdp_bpfmaps[i];
		}
    }
	return NULL;
}

static int trn_transit_map_get_fd(char *map_name)
{
	trn_xdp_map_t *xdpmap = trn_transit_map_get(map_name);
	if (!xdpmap) {
		TRN_LOG_INFO("Failed to find bpfmap %s.\n", map_name);
		return -1;
	}

	return xdpmap->fd;
}

/* Set up a hash lookup for bpfmaps */
static int trn_transit_map_hash(void)
{
	int num_maps = sizeof(trn_xdp_bpfmaps) / sizeof(trn_xdp_bpfmaps[0]);
	for (int i = 0; i < num_maps; i++) {
		trn_xdp_bpfmaps[i].fd = -1;
		trn_xdp_bpfmaps[i].map = NULL;
    }
	return 0;
}

static void trn_transit_map_hash_destroy(void)
{
}

/*
 * Setup bpfmap to use shared map if it was pinned   
 * Must be invoked before load to take effect
 */
static int trn_transit_xdp_pre_load(trn_prog_t *prog, int prog_idx)
{
	UNUSED(prog_idx);
	char buf[TRAN_MAX_PATH_SIZE];
	struct bpf_program *bpf_prog, *first_prog = NULL;
	struct bpf_object *obj;
	struct bpf_map *map;
	int len, fd;

	if (!prog || !prog->obj) {
		TRN_LOG_ERROR("Invalid input.\n");
		return 1;
	}

	obj = prog->obj;

	TRN_LOG_INFO("trn_transit_xdp_pre_load\n");

	/* Customize programs in object before loading */
	bpf_object__for_each_program(bpf_prog, obj) {
		bpf_program__set_type(bpf_prog, BPF_PROG_TYPE_XDP);
		bpf_program__set_ifindex(bpf_prog, 0);
		if (!first_prog) {
			first_prog = bpf_prog;
		}
	}

	if (!first_prog) {
		TRN_LOG_ERROR("Failed to find XDP program in object file: %s\n",
				bpf_object__name(obj));
		return 1;
	}

	/* Customize bpfmaps in object before loading */
	bpf_object__for_each_map(map, obj) {
		char *map_name = (char *)bpf_map__name(map);

		bpf_map__set_ifindex(map, 0);

		trn_xdp_map_t *xdpmap = trn_transit_map_get(map_name);

		if (!xdpmap) {
			TRN_LOG_INFO("Untracked obj bpfmap %s.\n", map_name);
			continue;
		}

		if (!xdpmap->map) {
			xdpmap->map = map;
		}

		/* Mark bpfmap for reuse if already shared */
		if (xdpmap->shared) {
			if (xdpmap->fd >= 0) {
				if (bpf_map__reuse_fd(map, xdpmap->fd)) {
					TRN_LOG_ERROR("Error reusing pinned map %s.\n",
						map_name);
					return 1;
				}
				TRN_LOG_INFO("Will reuse shared map %s.\n", map_name);
			} else {
				TRN_LOG_INFO("Will share %s map for reuse.\n", map_name);
			}
		}		
	}

	return 0;
}

/*
 * Pin bpfmap for sharing if it was not pinned  
 * Must be invoked AFTER load
 */
static int trn_transit_xdp_post_load(trn_prog_t *prog, int prog_idx)
{
	char buf[TRAN_MAX_PATH_SIZE];
	struct bpf_program *first_prog;
	struct bpf_object *obj;
	struct bpf_map *map;
	char *map_name, *pinfile;
	int len, fd;

	if (!prog || !prog->obj) {
		TRN_LOG_ERROR("Invalid input.\n");
		return 1;
	}

	obj = prog->obj;

	first_prog = bpf_program__next(NULL, obj);
	if (!first_prog) {
		TRN_LOG_ERROR("Failed to find XDP program in object file: %s\n",
				bpf_object__name(obj));
		return 1;
	}
	prog->prog_fd = bpf_program__fd(first_prog);

	TRN_LOG_INFO("trn_transit_xdp_post_load\n");

	bpf_object__for_each_map(map, obj) {
		map_name = (char *)bpf_map__name(map);

		trn_xdp_map_t *xdpmap = trn_transit_map_get(map_name);

		if (!xdpmap) {
			TRN_LOG_INFO("Untracked obj bpfmap %s.\n",
				map_name);
			continue;
		}

		/* Pin all bpfmaps if it has not been done */
		if (prog_idx == TRAN_TRANSIT_PROG &&
			!strcmp(map_name, "xdpcap_hook_map")) {
			pinfile = prog->pcapfile;
		} else {
			len = snprintf(buf, TRAN_MAX_PATH_SIZE, "%s/%s",
				pin_path, map_name);
			if (len < 0 || len >= TRAN_MAX_PATH_SIZE) {
				TRN_LOG_ERROR("Error generating pinned map path for %s.\n",
					map_name);
				return 1;
			}
			pinfile = buf;
		}

		fd = bpf_obj_get(pinfile);
		if (fd >= 0) {
			/* Already pinned, no-op */
			close(fd);
			TRN_LOG_INFO("Post: No-op for already pinned map %s.\n",
				map_name);
		} else if (bpf_map__pin(map, pinfile)) {
			TRN_LOG_ERROR("Error pinning map %s.\n", map_name);
			return 1;
		} else {
			TRN_LOG_INFO("Post: successfully pinned map %s.\n",	map_name);
		}

		if (xdpmap->shared && xdpmap->fd < 0) {
			xdpmap->fd = bpf_map__fd(map);
			TRN_LOG_INFO("Post: successfully shared map %s for reuse.\n",
				map_name);
		}
	}

	return 0;
}

/*
 * Pin bpfmap for sharing if it was not pinned  
 * Must be invoked AFTER load
 */
static int trn_transit_xdp_map_initialize(void)
{
	int fd, idx, err;

	if (!md) {
		TRN_LOG_ERROR("Invalid input.\n");
		return 1;
	}

	TRN_LOG_INFO("trn_transit_xdp_map_initialize\n");

	/* Initialize jmp_table */
	fd = trn_transit_map_get_fd("jmp_table");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get jmp_table fd");
		return 1;
	}
	for (idx = TRAN_TRANSIT_PROG + 1; idx < TRAN_MAX_PROG; idx++) {
		err = bpf_map_update_elem(fd, &idx, &md->ebpf_progs[idx].prog_fd, 0);
		if (err) {
			TRN_LOG_ERROR("Failed to add prog %d to jmp table (err:%d).",
				idx, err);
			return 1;
		}
	}

	/* Initialize interfaces_map for redirect */
	fd = trn_transit_map_get_fd("interfaces_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get fd for interfaces_map");
		return 0; // FIXME 1
	}
	for (idx = 0; idx < TRAN_ITF_MAP_MAX; idx++) {
		trn_iface_t *eth = &md->objs[idx].eth;
		err = bpf_map_update_elem(fd, &eth->role, &eth->iface_index, 0);
		if (err) {
			TRN_LOG_ERROR("Failed to update interfaces_map: %d - %d (err:%d).",
				eth->role, eth->iface_index, err);
			return 1;
		}
	}

	/* Don't initialize if_config_map untill droplets created */
	// Should initial with all zero config map to avoid garbage data lookup
	return 0;
}

static int trn_prog_load(trn_prog_t *prog, int prog_idx)
{
	struct bpf_object_open_attr open_attr = {
		.file = md->prog_tbl[prog_idx].prog_path,
		.prog_type = BPF_PROG_TYPE_XDP
	};

	if (!md) {
		TRN_LOG_ERROR("metadata not initialized while loading %s",
			open_attr.file);
		return 1;
	}

	if (prog->obj) {
		bpf_object__close(prog->obj);
	}

	/*
 	 * BIN_FIXME: update libbpf to leverage auto pinning
	 * Mimic bpf_prog_load_xattr() to enable map sharing
	 */
	TRN_LOG_INFO("trn_prog_load %s\n", open_attr.file);

	/* Step 1: Open bpf object to customize bpfmap sharing */
	prog->obj = bpf_object__open_xattr(&open_attr);
	if (IS_ERR_OR_NULL(prog->obj)) {
		TRN_LOG_ERROR("Error openning XDP object for %s\n", open_attr.file);
		return 1;
	}

	/* Step 2: Customize bpfpbg before loading */
	if (trn_transit_xdp_pre_load(prog, prog_idx)) {
		TRN_LOG_ERROR("Failed in pre-load processing for %s\n", open_attr.file);
		goto cleanup;
	}

	/* Step 3: Load object */
	if (bpf_object__load(prog->obj)) {
		TRN_LOG_ERROR("Error loading XDP object for %s\n", open_attr.file);
		goto cleanup;
	}

	/* Step 4: Save context and prepare for interface attachment */
	if (trn_transit_xdp_post_load(prog, prog_idx)) {
		TRN_LOG_ERROR("Failed in post-load processing for %s\n", open_attr.file);
		goto cleanup;
	}
	return 0;
cleanup:
	bpf_object__close(prog->obj);
	prog->obj = NULL;
	return 1;
}

int trn_update_dft(__u32 *dft_key, struct dft_t *dft)
{
	int fd = trn_transit_map_get_fd("dfts_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get dfts_map fd");
		return 1;
	}

	int err = bpf_map_update_elem(fd, dft_key, dft, 0);
	if (err) {
		TRN_LOG_ERROR("Store DFT mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_update_chain(__u32 *chain_key, struct chain_t *chain)
{
	int fd = trn_transit_map_get_fd("chains_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get chains_map fd");
		return 1;
	}

	int err = bpf_map_update_elem(fd, chain_key, chain, 0);
	if (err) {
		TRN_LOG_ERROR("Store Chain mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_update_ftn(__u32 *ftn_key, struct ftn_t *ftn)
{
	int fd = trn_transit_map_get_fd("ftns_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get ftns_map fd");
		return 1;
	}

	int err = bpf_map_update_elem(fd, ftn_key, ftn, 0);
	if (err) {
		TRN_LOG_ERROR("Store FTN mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_update_endpoints_get_ctx(void)
{
	int fd;

	fd = trn_transit_map_get_fd("endpoints_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get endpoints_map fd");
	}

	return fd;
}

int trn_update_endpoint(int fd, endpoint_key_t *epkey, endpoint_t *ep)
{
	int err;

	if (fd < 0) {
		TRN_LOG_ERROR("Invalid endpoints_map fd");
		return 1;
	}

	err = bpf_map_update_elem(fd, epkey, ep, 0);
	if (err) {
		TRN_LOG_ERROR("Store endpoint mapping failed (err:%d).", err);
		return 1;
	}

	return 0;
}

int trn_get_dft(__u32 *dft_key, struct dft_t *dft)
{
	int fd = trn_transit_map_get_fd("dfts_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get dfts_map fd");
		return 1;
	}

	int err = bpf_map_lookup_elem(fd, dft_key, dft);
	if (err) {
		TRN_LOG_ERROR("Querying DFT mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_get_chain(__u32 *chain_key, struct chain_t *chain)
{
	int fd = trn_transit_map_get_fd("chains_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get chains_map fd");
		return 1;
	}

	int err = bpf_map_lookup_elem(fd, chain_key, chain);
	if (err) {
		TRN_LOG_ERROR("Querying Chain mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_get_ftn(__u32 *ftn_key, struct ftn_t *ftn)
{
	int fd = trn_transit_map_get_fd("ftns_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get ftns_map fd");
		return 1;
	}

	int err = bpf_map_lookup_elem(fd, ftn_key, ftn);
	if (err) {
		TRN_LOG_ERROR("Querying FTN mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_get_endpoint(endpoint_key_t *epkey, endpoint_t *ep)
{
	int fd, err;

	fd = trn_transit_map_get_fd("endpoints_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get endpoints_map fd");
		return 1;
	}

	err = bpf_map_lookup_elem(fd, epkey, ep);
	if (err) {
		TRN_LOG_ERROR("Querying endpoint mapping failed (err:%d).",
			      err);
		return 1;
	}
	return 0;
}

int trn_remove_prog(unsigned int prog_idx)
{
	int fd = trn_transit_map_get_fd("jmp_table");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get jmp_table fd");
		return 1;
	}

	int err;
	err = bpf_map_delete_elem(fd, &prog_idx);
	if (err) {
		TRN_LOG_ERROR("Error add prog to trn jmp table (err:%d).", err);
	}
	bpf_object__close(md->ebpf_progs[prog_idx].obj);
	return 0;
}

int trn_delete_dft(__u32 *dft_key)
{
	int fd = trn_transit_map_get_fd("dfts_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get dfts_map fd");
		return 1;
	}

	int err = bpf_map_delete_elem(fd, dft_key);
	if (err) {
		TRN_LOG_ERROR("Deleting DFT mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_delete_chain(__u32 *chain_key)
{
	int fd = trn_transit_map_get_fd("chains_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get chains_map fd");
		return 1;
	}

	int err = bpf_map_delete_elem(fd, chain_key);
	if (err) {
		TRN_LOG_ERROR("Deleting Chain mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_delete_ftn(__u32 *ftn_key)
{
	int fd = trn_transit_map_get_fd("ftns_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get ftns_map fd");
		return 1;
	}

	int err = bpf_map_delete_elem(fd, ftn_key);
	if (err) {
		TRN_LOG_ERROR("Deleting FTN mapping failed (err:%d).", err);
		return 1;
	}
	return 0;
}

int trn_delete_endpoint(endpoint_key_t *epkey)
{
	endpoint_t ep;
	int fd, err;

	fd = trn_transit_map_get_fd("endpoints_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get endpoints_map fd");
		return 1;
	}

	err = bpf_map_lookup_elem(fd, epkey, &ep);

	if (err) {
		TRN_LOG_ERROR("Querying endpoint for delete failed (err:%d).",
			      err);
		return 1;
	}

	err = bpf_map_delete_elem(fd, epkey);
	if (err) {
		TRN_LOG_ERROR("Deleting endpoint mapping failed (err:%d).",
			      err);
		return 1;
	}

	return 0;
}

trn_iface_t *trn_get_itf_context(char *interface)
{
	unsigned int iface_index;

	if (!md) {
		TRN_LOG_ERROR("Userspace XDP metadata not initialized");
		return NULL;
	}

	iface_index = if_nametoindex(interface);
	if (!iface_index) {
		TRN_LOG_ERROR("if_nametoindex failed for %s", interface);
		return NULL;
	}

	for (int idx = 0; idx < TRAN_ITF_MAP_MAX; idx++) {
		if (md->objs[idx].eth.iface_index == iface_index) {
			return &md->objs[idx].eth;
		}
	}

	TRN_LOG_ERROR("Interface %s is not used for Transit XDP", interface);
	return NULL;
}

int trn_update_itf_config(struct tunnel_iface_t *itf)
{
	int	fd = trn_transit_map_get_fd("if_config_map");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get if_config_map fd");
		return 1;
	}

	int err = bpf_map_update_elem(fd, &itf->iface_index, itf, 0);
	if (err) {
		TRN_LOG_ERROR("Failed to update if_config_map: %d (err:%d).",
			itf->iface_index, err);
		return 1;
	}

	return 0;
}

/* Initialize Transit XDP Basic Objects */
// parameters: 
int trn_transit_xdp_load(char **interfaces, unsigned short ibo_port, bool debug)
{
	int i;
	struct rlimit r = { RLIM_INFINITY, RLIM_INFINITY };
	trn_prog_t *prog;

	TRN_LOG_INFO("Start loading Transit XDP in %sdebug mode.", debug? "":"non-");

	if (md) {
		TRN_LOG_INFO("Userspace XDP metadata already initialized, ignore");
		return 0;
	}
	
	if (setrlimit(RLIMIT_MEMLOCK, &r)) {
		TRN_LOG_ERROR("setrlimit(RLIMIT_MEMLOCK)");
		return 1;
	}

	/* Setting up a hash lookup for bpfmaps */
	if (trn_transit_map_hash()) {
		TRN_LOG_ERROR("Failed to create bpfmap hash table");
		return 1;
	}

	md = malloc(sizeof(user_metadata_t));
	if (!md) {
		TRN_LOG_ERROR("Failed to allocate userspace XDP metadata");
		return 1;
	}

	memset(md, 0, sizeof(user_metadata_t));

	// BIN_FIXME: consider NATIVE mode later
	md->xdp_flags = XDP_FLAGS_SKB_MODE;
	md->prog_tbl = debug?trn_prog_dbg_tbl:trn_prog_tbl;

	/* Step 1: Load Transit XDP object for interfaces attachment */
	for (i = 0; i < TRAN_ITF_MAP_MAX; i++) {
		trn_iface_t *eth = &md->objs[i].eth;

		eth->iface_index = if_nametoindex(interfaces[i]);
		if (!eth->iface_index) {
			TRN_LOG_ERROR("if_nametoindex failed for %s", interfaces[i]);
			goto cleanup;
		}
		eth->ibo_port = ibo_port;
		eth->protocol = trn_xdp_itf_def[i].itf_protocol;
		eth->role = trn_xdp_itf_def[i].itf_xdp_role;
		prog = &md->objs[i].xdp;
		snprintf(prog->pcapfile, sizeof(prog->pcapfile),
			"/sys/fs/bpf/transit_xdp_pcap_%s", interfaces[i]);

		/* Load transit XDP main program */
		if (trn_prog_load(prog, TRAN_TRANSIT_PROG)) {
			TRN_LOG_ERROR("Loading transit XDP program %d failed\n", TRAN_TRANSIT_PROG);
			goto cleanup;
		}
	}

	/* Step 2: Load Transit XDP sidecar eBPF program objects */
	for (i = TRAN_TRANSIT_PROG + 1; i < TRAN_MAX_PROG; i++) {
		prog = &md->ebpf_progs[i];
		if (trn_prog_load(prog, i)) {
			TRN_LOG_ERROR("Loading transit XDP program %d failed\n", i);
			goto cleanup;
		}
	}
	
	/* Initialize bpfmaps before attach to interfaces */
	if (trn_transit_xdp_map_initialize()) {
		TRN_LOG_ERROR("Failed to initialize bpfmaps before attach\n");
		goto cleanup;
	}

	/* Attach transit XDP main program to interfaces */
	for (i = 0; i < TRAN_ITF_MAP_MAX; i++) {
		struct bpf_prog_info info;
		__u32 info_len = sizeof(info);

		memset(&info, 0, info_len);

		/* Attach main transit XDP program to interface */
		if (bpf_set_link_xdp_fd(md->objs[i].eth.iface_index,
			md->objs[i].xdp.prog_fd, md->xdp_flags) < 0) {
			TRN_LOG_ERROR("Failed to attach XDP program %s to %s - %s\n",
				md->prog_tbl[TRAN_TRANSIT_PROG].prog_path, interfaces[i], strerror(errno));
			goto cleanup;
		}

		/* Store program id after attachment */
		if (bpf_obj_get_info_by_fd(md->objs[i].xdp.prog_fd, &info, &info_len)) {
			TRN_LOG_ERROR("Failed to get prog info - %s.", strerror(errno));
			goto cleanup;
		}
		md->objs[i].xdp.prog_id = info.id;

		TRN_LOG_INFO("Successfully loaded transit XDP on interface %s", interfaces[i]);
	}

	md->ready = TRUE;
	return 0;

cleanup:
	free(md);
	md = NULL;
	return 1;
}

int trn_transit_xdp_unload(char **interfaces)
{
	char buf[TRAN_MAX_PATH_SIZE];
	int i, len;

	TRN_LOG_INFO("Start unloading Transit XDP.");

	if (!md) {
		TRN_LOG_INFO("Userspace XDP metadata not initialized, ignore");
		return 0;
	}

	/* Step 1: Detatch XDP program from interfaces before releasing bpfmaps */
	for (i = 0; i < TRAN_ITF_MAP_MAX; i++) {
		__u32 link_prog_id = 0;

		if (bpf_get_link_xdp_id(md->objs[i].eth.iface_index, &link_prog_id, md->xdp_flags)) {
			TRN_LOG_ERROR("Failed to get XDP prog_id from %s", interfaces[i]);
			return 1;
		}

		if (md->objs[i].xdp.prog_id == link_prog_id) {
			bpf_set_link_xdp_fd(md->objs[i].eth.iface_index, -1, md->xdp_flags);
		} else if (!link_prog_id) {
			TRN_LOG_WARN("couldn't find a prog id on %s\n", interfaces[i]);
		} else {
			TRN_LOG_WARN("program on %s changed, not removing\n", interfaces[i]);
		}

		if (bpf_set_link_xdp_fd(md->objs[i].eth.iface_index,
			md->objs[i].xdp.prog_fd, md->xdp_flags) < 0) {
			TRN_LOG_ERROR("Failed to attach XDP program %s to %s - %s\n",
				md->prog_tbl[TRAN_TRANSIT_PROG].prog_path, interfaces[i], strerror(errno));
			return 1;
		}
	}

	/* Step 2: Un-pin pinned maps */
	int num_maps = sizeof(trn_xdp_bpfmaps) / sizeof(trn_xdp_bpfmaps[0]);
	for (i = 0; i < num_maps; i++) {

		if (trn_xdp_bpfmaps[i].shared) {
			len = snprintf(buf, TRAN_MAX_PATH_SIZE,
				"%s/%s", pin_path, trn_xdp_bpfmaps[i].name);
			if (len < 0 || len >= TRAN_MAX_PATH_SIZE) {
				TRN_LOG_ERROR("Error generating pinned map path for %s.\n",
					trn_xdp_bpfmaps[i].name);
				return 1;
			}
			bpf_map__unpin(trn_xdp_bpfmaps[i].map, buf);
		} else if (!strcmp(trn_xdp_bpfmaps[i].name, "xdpcap_hook_map")) {
			bpf_map__unpin(trn_xdp_bpfmaps[i].map, md->objs[i].xdp.pcapfile);
		}
    }
	TRN_LOG_ERROR("trn_transit_xdp_unload, destroy hash");
	trn_transit_map_hash_destroy();

	/* Step 3: Close bpfobjs */
	for (i = 1; i < TRAN_MAX_PROG; i++) {
		if (!IS_ERR_OR_NULL(md->ebpf_progs[i].obj)) {
			bpf_object__close(md->ebpf_progs[i].obj);
		}
	}
	for (i = 0; i < TRAN_ITF_MAP_MAX; i++) {
		bpf_object__close(md->objs[i].xdp.obj);
	}

	free(md);
	md = NULL;
	return 0;
}

int trn_transit_ebpf_load(int prog_idx)
{
	trn_prog_t *prog;
	int err, fd;

	TRN_LOG_INFO("Start loading eBPF program %d.", prog_idx);

	if (!md) {
		TRN_LOG_ERROR("Userspace XDP metadata not initialized");
		return 1;
	}

	if (prog_idx <= TRAN_TRANSIT_PROG || prog_idx >= TRAN_MAX_PROG) {
		TRN_LOG_ERROR("Invalid program index");
		return 1;
	}

	prog = &md->ebpf_progs[prog_idx];

	/* Load eBPF program */
	if (trn_prog_load(prog, prog_idx)) {
		TRN_LOG_ERROR("Loading transit XDP program %d failed\n", prog_idx);
		return 1;
	}

	/* Add eBPF program to junp_table */
	fd = trn_transit_map_get_fd("jmp_table");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get jmp_table fd");
		return 1;
	}
	err = bpf_map_update_elem(fd, &prog_idx, &md->ebpf_progs[prog_idx].prog_fd, 0);
	if (err) {
		TRN_LOG_ERROR("Failed to add prog %d to jmp table (err:%d).",
			prog_idx, err);
		return 1;
	}

	return 0;
}

int trn_transit_ebpf_unload(int prog_idx)
{
	int err, fd;

	TRN_LOG_INFO("Start unloading eBPF program %d.", prog_idx);

	if (!md) {
		TRN_LOG_ERROR("Userspace XDP metadata not initialized");
		return 1;
	}

	if (prog_idx <= TRAN_TRANSIT_PROG || prog_idx >= TRAN_MAX_PROG) {
		TRN_LOG_ERROR("Invalid program index");
		return 1;
	}

	fd = trn_transit_map_get_fd("jmp_table");
	if (fd < 0) {
		TRN_LOG_ERROR("Failed to get jmp_table fd");
		return 1;
	}
	err = bpf_map_delete_elem(fd, &prog_idx);
	if (err) {
		TRN_LOG_ERROR("Error removing prog %d from jmp table (err:%d).",
			prog_idx, err);
	}

	bpf_object__close(md->ebpf_progs[prog_idx].obj);

	md->ebpf_progs[prog_idx].obj = NULL;
	close(md->ebpf_progs[prog_idx].prog_fd);
	md->ebpf_progs[prog_idx].prog_fd = -1;
	return 0;
}

int trn_transit_dp_assistant(void)
{
	flow_ctx_t sendbuf;
	int sockfd, oam_fd, rc;
	struct sockaddr_ll socket_address;
	__u32 if_idx;

	/* Wait for Transit XDP provisioned */
	while (!md || !md->ready) {
		//usleep(500*1000);
		nanosleep((const struct timespec[]){{0, 500*1000*1000L}}, NULL);
	}
	TRN_LOG_INFO("DPA ready!");

	/* Grab the tx interface index */
	if_idx = md->objs[XDP_FWD].eth.iface_index;

	/* Grab the oam_queue_map handle */
	oam_fd = trn_transit_map_get_fd("oam_queue_map");

	/* Open RAW socket to send on */
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    TRN_LOG_ERROR("DPA failed to open raw socket!");
		return 1;
	}

	for(;;){
		rc = bpf_map_lookup_and_delete_elem(oam_fd, NULL, (void *)&sendbuf);
		if (rc) {
			/* No more to send */
			//usleep(500);
			nanosleep((const struct timespec[]){{0, 500*1000L}}, NULL);
		} else {
			/* OAM packet in buffer, ship it */
			socket_address.sll_ifindex = if_idx;
			socket_address.sll_halen = ETH_ALEN;
			/* Destination MAC */
			memcpy(socket_address.sll_addr, sendbuf.eth.h_dest,
				sizeof(sendbuf.eth.h_dest));
			if (sendto(sockfd, &sendbuf.eth, sendbuf.len, 0,
				(struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
				TRN_LOG_ERROR("DPA failed to send oam packet to 0x%08x.",
					sendbuf.ip.daddr);
			}
		}
	}

	return 0;
}
