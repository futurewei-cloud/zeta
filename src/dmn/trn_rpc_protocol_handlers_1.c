// SPDX-License-Identifier: GPL-2.0
/**
 * @file trn_rpc_protocol_handlers_1.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief RPC handlers. Primarly allocate and populate data structs,
 * and update the ebpf maps through user space APIs.
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
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <search.h>
#include <stdlib.h>
#include <stdint.h>

#include "trn_transitd.h"

void rpc_transit_remote_protocol_1(struct svc_req *rqstp,
				   register SVCXPRT *transp);

int *update_dft_1_svc(rpc_trn_dft_t *dft, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	__u32 dft_key;
	struct dft_t dft_val;

	TRN_LOG_DEBUG("update_dft_1 dft id: %d", dft->id);

	dft_key = dft->id;
	dft_val.table_len = dft->table.table_len;
	if (dft_val.table_len > TRAN_MAX_MAGLEV_TABLE_SIZE) {
		TRN_LOG_WARN(
			"Number of maximum remote Table entries exceeded %d!",
			TRAN_MAX_MAGLEV_TABLE_SIZE);
		result = RPC_TRN_ERROR;
		goto error;
	}

	if (dft_val.table_len > 0) {
		memcpy(dft_val.table, dft->table.table_val,
		       dft_val.table_len * sizeof(dft_val.table[0]));
	}
	rc = trn_update_dft(&dft_key, &dft_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot update transit XDP with dft %d", dft_key);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;

error:
	return &result;
}

int *update_chain_1_svc(rpc_trn_chain_t *chain, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	__u32 chain_key;
	struct chain_t chain_val;

	TRN_LOG_DEBUG("update_chain_1 chain id: %d", chain->id);

	chain_key = chain->id;

	chain_val.tail_ftn = chain->tail_ftn;
	chain_val.tail_ftn_ip = chain->tail_ftn_ip;
	memcpy(chain_val.tail_ftn_mac, chain->tail_ftn_mac,
	       6 * sizeof(chain_val.tail_ftn_mac[0]));
	rc = trn_update_chain(&chain_key, &chain_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot update transit XDP with chain %d",
			      chain->id);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;

error:
	return &result;
}

int *update_ftn_1_svc(rpc_trn_ftn_t *ftn, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	__u32 ftn_key;
	struct ftn_t ftn_val;

	TRN_LOG_DEBUG("update_ftn_1 ftn id: %d", ftn->id);

	ftn_key = ftn->id;

	ftn_val.position = ftn->position;
	ftn_val.ip = ftn->ip;
	ftn_val.next_ip = ftn->next_ip;
	memcpy(ftn_val.mac, ftn->mac, 6 * sizeof(ftn_val.mac[0]));
	memcpy(ftn_val.next_mac, ftn->next_mac,
	       6 * sizeof(ftn_val.next_mac[0]));

	rc = trn_update_ftn(&ftn_key, &ftn_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot update transit XDP with ftn %d", ftn->id);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;

error:
	return &result;
}

int *update_ep_1_svc(rpc_trn_endpoint_batch_t *batch, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc, ctx;

	trn_ep_t *ep = (trn_ep_t *)batch->rpc_trn_endpoint_batch_t_val;

	TRN_LOG_DEBUG("update_ep_1 batch size: %d",
		      batch->rpc_trn_endpoint_batch_t_len);

	ctx = trn_update_endpoints_get_ctx();
	if (ctx < 0) {
		TRN_LOG_ERROR("Failed to get update endpoints context");
		result = RPC_TRN_ERROR;
		goto error;
	}

	for (__u32 i = 0; i < batch->rpc_trn_endpoint_batch_t_len; i++, ep++) {
		rc = trn_update_endpoint(ctx, &ep->xdp_ep.key, &ep->xdp_ep.val);
		if (rc) {
			TRN_LOG_ERROR("Failed to update endpoint %d %08x",
				      ep->xdp_ep.key.vni, ep->xdp_ep.key.ip);
			result = RPC_TRN_ERROR;
			goto error;
		}
	}
	result = 0;

error:
	return &result;
}

int *delete_dft_1_svc(rpc_trn_zeta_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	__u32 dft_key;

	TRN_LOG_DEBUG("delete_dft_1 dft id: %d", argp->id);

	dft_key = argp->id;
	rc = trn_delete_dft(&dft_key);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting dft %d", argp->id);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;
error:
	return &result;
}

int *delete_chain_1_svc(rpc_trn_zeta_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	__u32 chain_key;

	TRN_LOG_DEBUG("delete_chain_1 chain id: %d", argp->id);

	chain_key = argp->id;
	rc = trn_delete_chain(&chain_key);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting chain %d", argp->id);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;
error:
	return &result;
}

int *delete_ftn_1_svc(rpc_trn_zeta_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	__u32 ftn_key;

	TRN_LOG_DEBUG("delete_ftn_1 ftn id: %d", argp->id);

	ftn_key = argp->id;
	rc = trn_delete_ftn(&ftn_key);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting ftn %d", argp->id);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;
error:
	return &result;
}

int *delete_ep_1_svc(rpc_endpoint_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;

	TRN_LOG_DEBUG("delete_ep_1 ep vni: %ld, ip: 0x%x", argp->vni, argp->ip);

	rc = trn_delete_endpoint((endpoint_key_t *)argp);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting ep %d - %d", argp->vni,
			      argp->ip);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;
error:
	return &result;
}

rpc_trn_dft_t *get_dft_1_svc(rpc_trn_zeta_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_dft_t result;
	result.table.table_len = 0;
	int rc;
	__u32 dft_key;
	static struct dft_t dft_val;

	TRN_LOG_DEBUG("get_dft_1 dft id: %d", argp->id);

	dft_key = argp->id;
	rc = trn_get_dft(&dft_key, &dft_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot get dft %d", argp->id);
		goto error;
	}
	result.id = argp->id;
	result.table.table_len = dft_val.table_len;
	result.table.table_val = dft_val.table;
	return &result;

error:
	return NULL;
}

rpc_trn_chain_t *get_chain_1_svc(rpc_trn_zeta_key_t *argp,
				 struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_chain_t result;
	int rc;
	__u32 chain_key;
	static struct chain_t chain_val;

	TRN_LOG_DEBUG("get_chain_1 chain id: %d", argp->id);

	chain_key = argp->id;
	rc = trn_get_chain(&chain_key, &chain_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot get chain %d", argp->id);
		goto error;
	}
	result.id = argp->id;
	result.tail_ftn = chain_val.tail_ftn;
	result.tail_ftn_ip = chain_val.tail_ftn_ip;
	memcpy(result.tail_ftn_mac, chain_val.tail_ftn_mac,
	       sizeof(chain_val.tail_ftn_mac));

	return &result;

error:
	return NULL;
}

rpc_trn_ftn_t *get_ftn_1_svc(rpc_trn_zeta_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_ftn_t result;
	memset(result.mac, 0, sizeof(result.mac));
	memset(result.next_mac, 0, sizeof(result.next_mac));
	int rc;
	__u32 ftn_key;
	static struct ftn_t ftn_val;

	TRN_LOG_DEBUG("get_ftn_1 ftn id: %d", argp->id);

	ftn_key = argp->id;
	rc = trn_get_ftn(&ftn_key, &ftn_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot get ftn %d", argp->id);
		goto error;
	}
	result.id = argp->id;
	result.position = ftn_val.position;
	result.ip = ftn_val.ip;
	result.next_ip = ftn_val.next_ip;
	memcpy(result.mac, ftn_val.mac, sizeof(ftn_val.mac));
	memcpy(result.next_mac, ftn_val.next_mac, sizeof(ftn_val.next_mac));
	return &result;

error:
	return NULL;
}

rpc_trn_endpoint_t *get_ep_1_svc(rpc_endpoint_key_t *argp,
				 struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static trn_ep_t result;
	int rc;

	TRN_LOG_DEBUG("get_ep_1 ep vni: %ld, ip: 0x%x", argp->vni, argp->ip);

	rc = trn_get_endpoint((endpoint_key_t *)argp, &result.xdp_ep.val);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot find ep %d - %d from XDP map", argp->vni,
			      argp->ip);
		goto error;
	}

	result.xdp_ep.key.vni = argp->vni;
	result.xdp_ep.key.ip = argp->ip;

	return &result.rpc_ep;

error:
	return NULL;
}

int *update_droplet_1_svc(rpc_trn_droplet_t *droplet, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	struct tunnel_iface_t itf;
	trn_iface_t *eth;

	eth = trn_get_itf_context(droplet->interface);
	if (!eth) {
		TRN_LOG_ERROR("Failed to get droplet interface context %s",
			      droplet->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	for (unsigned int i = 0; i < droplet->num_entrances; i++) {
		itf.entrances[i].ip = droplet->entrances[i].ip;
		memcpy(itf.entrances[i].mac, droplet->entrances[i].mac,
		       sizeof(droplet->entrances[i].mac));
		itf.entrances[i].announced = 0;
	}
	itf.num_entrances = droplet->num_entrances;
	itf.iface_index = eth->iface_index;
	itf.ibo_port = eth->ibo_port;
	itf.role = eth->role;
	itf.protocol = eth->protocol;

	rc = trn_update_itf_config(&itf);
	if (rc) {
		TRN_LOG_ERROR("Failed to update droplet %s",
			      droplet->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}
	result = 0;

error:
	return &result;
}

/* RPC backend to load transit XDP and attach to interfaces */
int *load_transit_xdp_1_svc(rpc_trn_xdp_intf_t *xdp_intf, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	bool debug = xdp_intf->debug_mode == 0 ? false : true;

	if (trn_transit_xdp_load(xdp_intf->interfaces, xdp_intf->ibo_port,
				 debug)) {
		TRN_LOG_ERROR("Failed to load transit XDP");
		result = RPC_TRN_FATAL;
	} else {
		result = 0;
	}

	return &result;
}

/* RPC backend to unload transit XDP from interfaces */
int *unload_transit_xdp_1_svc(rpc_trn_xdp_intf_t *xdp_intf,
			      struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;

	if (trn_transit_xdp_unload(xdp_intf->interfaces)) {
		TRN_LOG_ERROR("Failed to unload transit XDP");
		result = RPC_TRN_FATAL;
	} else {
		result = 0;
	}

	return &result;
}

int *load_transit_xdp_ebpf_1_svc(rpc_trn_ebpf_prog_t *argp,
				 struct svc_req *rqstp)
{
	UNUSED(rqstp);

	static int result;
	int rc;

	rc = trn_transit_ebpf_load(argp->prog_idx);

	if (rc != 0) {
		TRN_LOG_ERROR("Failed to insert XDP stage %d", argp->prog_idx);
		result = RPC_TRN_ERROR;
	} else {
		result = 0;
	}

	return &result;
}

int *unload_transit_xdp_ebpf_1_svc(rpc_trn_ebpf_prog_t *argp,
				   struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;

	rc = trn_transit_ebpf_unload(argp->prog_idx);

	if (rc != 0) {
		TRN_LOG_ERROR("Failed to remove XDP stage %d", argp->prog_idx);
		result = RPC_TRN_ERROR;
	} else {
		result = 0;
	}

	return &result;
}
