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

#include "rpcgen/trn_rpc_protocol.h"
#include "trn_transit_xdp_usr.h"
#include "trn_log.h"
#include "trn_transitd.h"

#define TRANSITLOGNAME "transit"
#define TRN_MAX_ITF 265
#define TRN_MAX_VETH 2048

void rpc_transit_remote_protocol_1(struct svc_req *rqstp,
				   register SVCXPRT *transp);

int trn_itf_table_init()
{
	int rc;
	rc = hcreate((TRN_MAX_VETH + TRN_MAX_ITF) * 1.3);
	return rc;
}

void trn_itf_table_free()
{
	/* TODO: At the moment, this is only called before exit, so there
     *  is no actual need to free table elements one by one. If this
     *  is being called while the dameon remains running, we will need
     *  to maintain the keys in a separate data-structure and free
     *  them one-by-one. */

	hdestroy();
}

int trn_itf_table_insert(char *itf, struct user_metadata_t *md)
{
	INTF_INSERT();
}

struct user_metadata_t *trn_itf_table_find(char *itf)
{
	INTF_FIND();
}

void trn_itf_table_delete(char *itf)
{
	INTF_DELETE();
}

int *update_dft_1_svc(rpc_trn_dft_t *dft, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	result = 0;
	int rc;
	char *itf = dft->interface;
	struct zeta_key_t dft_key;
	struct dft_t dft_val;

	TRN_LOG_DEBUG("update_dft_1 dft id: %d", dft->id);

	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	dft_key.id = dft->id;
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
	rc = trn_update_dft(md, &dft_key, &dft_val);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Cannot update transit XDP with dft %d on interface %s",
			dft_key.id, itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;

error:
	return &result;
}

int *update_chain_1_svc(rpc_trn_chain_t *chain, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	result = 0;
	int rc;
	char *itf = chain->interface;
	struct zeta_key_t chain_key;
	struct chain_t chain_val;

	TRN_LOG_DEBUG("update_chain_1 chain id: %d", chain->id);

	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	chain_key.id = chain->id;

	chain_val.tail_ftn = chain->tail_ftn;

	rc = trn_update_chain(md, &chain_key, &chain_val);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Cannot update transit XDP with chain %d on interface %s",
			chain->id, itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;

error:
	return &result;
}

int *update_ftn_1_svc(rpc_trn_ftn_t *ftn, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	result = 0;
	int rc;
	char *itf = ftn->interface;
	struct zeta_key_t ftn_key;
	struct ftn_t ftn_val;

	TRN_LOG_DEBUG("update_ftn_1 ftn id: %d", ftn->id);

	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	ftn_key.id = ftn->id;

	ftn_val.position = ftn->position;
	ftn_val.ip = ftn->ip;
	ftn_val.next_ip = ftn->next_ip;
	memcpy(ftn_val.mac, ftn->mac, 6 * sizeof(ftn_val.mac[0]));
	memcpy(ftn_val.next_mac, ftn->next_mac,
	       6 * sizeof(ftn_val.next_mac[0]));

	rc = trn_update_ftn(md, &ftn_key, &ftn_val);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Cannot update transit XDP with ftn %d on interface %s",
			ftn->id, itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;

error:
	return &result;
}

int *update_ep_1_svc(rpc_trn_endpoint_t *ep, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	result = 0;
	int rc;
	char *itf = ep->interface;
	struct endpoint_key_t epkey;
	struct endpoint_t epval;

	TRN_LOG_DEBUG("update_ep_1 ep tunid: %ld, ip: 0x%x,"
		      " type: %d, veth: %s, hosted_interface:%s",
		      ep->tunid, ep->ip, ep->eptype, ep->veth,
		      ep->hosted_interface);

	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	memcpy(epkey.tunip, &ep->tunid, sizeof(ep->tunid));
	epkey.tunip[2] = ep->ip;
	epval.eptype = ep->eptype;

	epval.nremote_ips = ep->remote_ips.remote_ips_len;
	if (epval.nremote_ips > TRAN_MAX_REMOTES) {
		TRN_LOG_WARN("Number of maximum remote IPs exceeded,"
			     " configuring only the first %d remote IPs.",
			     TRAN_MAX_REMOTES);
		result = RPC_TRN_WARN;
		goto error;
	}

	if (epval.nremote_ips > 0) {
		memcpy(epval.remote_ips, ep->remote_ips.remote_ips_val,
		       epval.nremote_ips * sizeof(epval.remote_ips[0]));
	}
	memcpy(epval.mac, ep->mac, 6 * sizeof(epval.mac[0]));

	if (strcmp(ep->hosted_interface, "") != 0) {
		epval.hosted_iface = if_nametoindex(ep->hosted_interface);
	} else {
		epval.hosted_iface = -1;
	}

	/* in case if_nameindex fails!! */
	if (!epval.hosted_iface) {
		TRN_LOG_WARN(
			"**Failed to map interface name [%s] to an index,"
			" for the given hosted_interface name traffic may"
			" not be routed correctly to ep [0x%x] hosted on this host.",
			ep->hosted_interface, ep->ip);
		result = RPC_TRN_WARN;
		epval.hosted_iface = -1;
	}

	rc = trn_update_endpoint(md, &epkey, &epval);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Cannot update transit XDP with ep %d on interface %s",
			epkey.tunip[2], itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;

error:
	return &result;
}

int *delete_dft_1_svc(rpc_trn_zeta_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	result = 0;
	int rc;
	struct zeta_key_t dft_key;

	TRN_LOG_DEBUG("delete_dft_1 dft id: %d on interface: %s", argp->id,
		      argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	dft_key.id = argp->id;
	rc = trn_delete_dft(md, &dft_key);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting dft %d on interface %s",
			      argp->id, argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;
error:
	return &result;
}

int *delete_chain_1_svc(rpc_trn_zeta_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	result = 0;
	int rc;
	struct zeta_key_t chain_key;

	TRN_LOG_DEBUG("delete_chain_1 chain id: %d on interface: %s", argp->id,
		      argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	chain_key.id = argp->id;
	rc = trn_delete_chain(md, &chain_key);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting chain %d on interface %s",
			      argp->id, argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;
error:
	return &result;
}

int *delete_ftn_1_svc(rpc_trn_zeta_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	result = 0;
	int rc;
	struct zeta_key_t ftn_key;

	TRN_LOG_DEBUG("delete_ftn_1 ftn id: %d on interface: %s", argp->id,
		      argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	ftn_key.id = argp->id;
	rc = trn_delete_ftn(md, &ftn_key);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting ftn %d on interface %s",
			      argp->id, argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;
error:
	return &result;
}

int *delete_ep_1_svc(rpc_trn_endpoint_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	result = 0;
	int rc;
	struct endpoint_key_t epkey;
	char buffer[INET_ADDRSTRLEN];
	const char *parsed_ip =
		inet_ntop(AF_INET, &argp->ip, buffer, sizeof(buffer));

	TRN_LOG_DEBUG("delete_ep_1 ep tunid: %ld, ip: 0x%x,"
		      " on interface: %s",
		      argp->tunid, parsed_ip, argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	memcpy(epkey.tunip, &argp->tunid, sizeof(argp->tunid));
	epkey.tunip[2] = argp->ip;

	rc = trn_delete_endpoint(md, &epkey);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting ep %d on interface %s",
			      epkey.tunip[2], argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	return &result;
error:
	return &result;
}

rpc_trn_dft_t *get_dft_1_svc(rpc_trn_zeta_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_dft_t result;
	result.table.table_len = 0;
	int rc;
	struct zeta_key_t dft_key;
	static struct dft_t dft_val;

	TRN_LOG_DEBUG("get_dft_1 dft id: %d on interface: %s", argp->id,
		      argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}

	dft_key.id = argp->id;
	rc = trn_get_dft(md, &dft_key, &dft_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot get dft %d on interface %s", argp->id,
			      argp->interface);
		goto error;
	}
	result.interface = argp->interface;
	result.id = argp->id;
	result.table.table_len = dft_val.table_len;
	result.table.table_val = dft_val.table;
	return &result;

error:
	result.interface = "";
	return &result;
}

rpc_trn_chain_t *get_chain_1_svc(rpc_trn_zeta_key_t *argp,
				 struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_chain_t result;
	int rc;
	struct zeta_key_t chain_key;
	static struct chain_t chain_val;

	TRN_LOG_DEBUG("get_chain_1 chain id: %d on interface: %s", argp->id,
		      argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}

	chain_key.id = argp->id;
	rc = trn_get_chain(md, &chain_key, &chain_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot get chain %d on interface %s", argp->id,
			      argp->interface);
		goto error;
	}
	result.interface = argp->interface;
	result.id = argp->id;
	result.tail_ftn = chain_val.tail_ftn;

	return &result;

error:
	result.interface = "";
	return &result;
}

rpc_trn_ftn_t *get_ftn_1_svc(rpc_trn_zeta_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_ftn_t result;
	memset(result.mac, 0, sizeof(result.mac));
	memset(result.next_mac, 0, sizeof(result.next_mac));
	int rc;
	struct zeta_key_t ftn_key;
	static struct ftn_t ftn_val;

	TRN_LOG_DEBUG("get_ftn_1 ftn id: %d on interface: %s", argp->id,
		      argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}

	ftn_key.id = argp->id;
	rc = trn_get_ftn(md, &ftn_key, &ftn_val);

	if (rc != 0) {
		TRN_LOG_ERROR("Cannot get ftn %d on interface %s", argp->id,
			      argp->interface);
		goto error;
	}
	result.interface = argp->interface;
	result.id = argp->id;
	result.position = ftn_val.position;
	result.ip = ftn_val.ip;
	result.next_ip = ftn_val.next_ip;
	memcpy(result.mac, ftn_val.mac, sizeof(ftn_val.mac));
	memcpy(result.next_mac, ftn_val.next_mac, sizeof(ftn_val.next_mac));
	return &result;

error:
	result.interface = "";
	return &result;
}

rpc_trn_endpoint_t *get_ep_1_svc(rpc_trn_endpoint_key_t *argp,
				 struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static rpc_trn_endpoint_t result;
	result.remote_ips.remote_ips_len = 0;
	result.hosted_interface = "";
	memset(result.mac, 0, sizeof(result.mac));
	int rc;
	struct endpoint_key_t epkey;
	static struct endpoint_t epval;

	TRN_LOG_DEBUG("get_ep_1 ep tunid: %ld, ip: 0x%x,"
		      " on interface: %s",
		      argp->tunid, argp->ip, argp->interface);

	struct user_metadata_t *md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}

	memcpy(epkey.tunip, &argp->tunid, sizeof(argp->tunid));
	epkey.tunip[2] = argp->ip;

	rc = trn_get_endpoint(md, &epkey, &epval);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Cannot update transit XDP with ep %d on interface %s",
			epkey.tunip[2], argp->interface);
		goto error;
	}

	result.interface = argp->interface;
	char buf[IF_NAMESIZE];
	result.tunid = argp->tunid;
	result.ip = argp->ip;
	if (epval.hosted_iface != -1) {
		result.hosted_interface =
			if_indextoname(epval.hosted_iface, buf);
		if (result.hosted_interface == NULL) {
			TRN_LOG_ERROR(
				"The interface at index %d does not exist.",
				epval.hosted_iface);
			goto error;
		}
	} else {
		result.hosted_interface = "";
	}
	result.eptype = epval.eptype;
	memcpy(result.mac, epval.mac, sizeof(epval.mac));
	result.remote_ips.remote_ips_len = epval.nremote_ips;
	result.remote_ips.remote_ips_val = epval.remote_ips;
	result.veth = ""; // field to be removed
	return &result;

error:
	result.interface = "";
	return &result;
}

int *load_transit_xdp_1_svc(rpc_trn_xdp_intf_t *xdp_intf, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;

	int rc;
	bool unload_error = false;
	char *itf = xdp_intf->interface;
	char *kern_path = xdp_intf->xdp_path;
	struct user_metadata_t empty_md;
	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (md) {
		TRN_LOG_INFO("meatadata for interface %s already exist.", itf);
	} else {
		TRN_LOG_INFO("creating meatadata for interface %s.", itf);
		md = malloc(sizeof(struct user_metadata_t));
	}

	if (!md) {
		TRN_LOG_ERROR("Failure allocating memory for user_metadata_t");
		result = RPC_TRN_FATAL;
		goto error;
	}

	memset(md, 0, sizeof(struct user_metadata_t));

	// Set all interface index slots to unused
	int i;
	for (i = 0; i < TRAN_MAX_ITF; i++) {
		md->itf_idx[i] = TRAN_UNUSED_ITF_IDX;
	}

	strcpy(md->pcapfile, xdp_intf->pcapfile);
	md->pcapfile[255] = '\0';
	md->xdp_flags = XDP_FLAGS_SKB_MODE;

	TRN_LOG_DEBUG("load_transit_xdp_1 path: %s, pcap: %s",
		      xdp_intf->xdp_path, xdp_intf->pcapfile);

	rc = trn_user_metadata_init(md, itf, kern_path, md->xdp_flags);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Failure initializing or loading transit XDP program for interface %s",
			itf);
		result = RPC_TRN_FATAL;
		goto error;
	}

	rc = trn_itf_table_insert(itf, md);
	if (rc != 0) {
		TRN_LOG_ERROR(
			"Failure populating interface table when loading XDP program on %s",
			itf);
		result = RPC_TRN_ERROR;
		unload_error = true;
		goto error;
	}

	TRN_LOG_INFO("Successfully loaded transit XDP on interface %s", itf);

	result = 0;
	return &result;

error:
	if (unload_error) {
		trn_user_metadata_free(md);
	}
	free(md);
	return &result;
}

int *unload_transit_xdp_1_svc(rpc_intf_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	char *itf = argp->interface;

	TRN_LOG_DEBUG("unload_transit_xdp_1 interface: %s", itf);

	struct user_metadata_t *md = trn_itf_table_find(itf);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s", itf);
		result = RPC_TRN_ERROR;
		goto error;
	}

	rc = trn_user_metadata_free(md);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Cannot free XDP metadata, transit program may still be running");
		result = RPC_TRN_ERROR;
		goto error;
	}
	trn_itf_table_delete(itf);

	result = 0;
	return &result;

error:
	return &result;
}

int *load_transit_xdp_pipeline_stage_1_svc(rpc_trn_ebpf_prog_t *argp,
					   struct svc_req *rqstp)
{
	UNUSED(rqstp);

	static int result;
	int rc;
	struct user_metadata_t *md;
	char *prog_path = argp->xdp_path;
	unsigned int prog_idx = argp->stage;

	switch (prog_idx) {
	case ON_XDP_TX:
	case ON_XDP_PASS:
	case ON_XDP_REDIRECT:
	case ON_XDP_DROP:
	case ON_XDP_SCALED_EP:
		break;
	default:
		TRN_LOG_ERROR("Unsupported program stage %s", argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}

	rc = trn_add_prog(md, prog_idx, prog_path);

	if (rc != 0) {
		TRN_LOG_ERROR("Failed to insert XDP stage %d for interface %s",
			      prog_idx, argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}

int *unload_transit_xdp_pipeline_stage_1_svc(rpc_trn_ebpf_prog_stage_t *argp,
					     struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	struct user_metadata_t *md;
	unsigned int prog_idx = argp->stage;

	switch (prog_idx) {
	case ON_XDP_TX:
	case ON_XDP_PASS:
	case ON_XDP_REDIRECT:
	case ON_XDP_DROP:
	case ON_XDP_SCALED_EP:
		break;
	default:
		TRN_LOG_ERROR("Unsupported program stage %s", argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	md = trn_itf_table_find(argp->interface);

	if (!md) {
		TRN_LOG_ERROR("Cannot find interface metadata for %s",
			      argp->interface);
		goto error;
	}

	rc = trn_remove_prog(md, prog_idx);

	if (rc != 0) {
		TRN_LOG_ERROR("Failed to remove XDP stage %d for interface %s",
			      prog_idx, argp->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;
	return &result;

error:
	return &result;
}
