// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_ep.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief CLI subcommands related to endpoints
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
#include "trn_cli.h"

int trn_cli_parse_ep_key(const cJSON *jsonobj,
			 struct rpc_trn_endpoint_key_t *ep)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");

	if (tunnel_id == NULL) {
		ep->tunid = 0;
		print_err("Warning: Tunnel ID default is used: %ld\n",
			  ep->tunid);
	} else if (cJSON_IsString(tunnel_id)) {
		ep->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Tunnel ID Error\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip->valuestring, &(sa.sin_addr));
		ep->ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: IP is missing or non-string\n");
		return -EINVAL;
	}

	return 0;
}

int trn_cli_parse_ep(const cJSON *jsonobj, struct rpc_trn_endpoint_t *ep)
{
	cJSON *tunnel_id = cJSON_GetObjectItem(jsonobj, "tunnel_id");
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");
	cJSON *eptype = cJSON_GetObjectItem(jsonobj, "eptype");
	cJSON *mac = cJSON_GetObjectItem(jsonobj, "mac");
	cJSON *veth = cJSON_GetObjectItem(jsonobj, "veth");
	cJSON *remote_ips = cJSON_GetObjectItem(jsonobj, "remote_ips");
	cJSON *remote_ip = NULL;
	cJSON *hosted_iface = cJSON_GetObjectItem(jsonobj, "hosted_iface");

	if (veth == NULL) {
		ep->veth = NULL;
		print_err("Warning: missing veth.\n");
	} else if (cJSON_IsString(
			   veth)) { // This transit is hosting the endpoint
		strcpy(ep->veth, veth->valuestring);
	}

	if (hosted_iface == NULL) {
		ep->hosted_interface = NULL;
		print_err(
			"Warning: missing hosted interface, using default.\n");
	} else if (cJSON_IsString(
			   hosted_iface)) { // This transit is hosting the endpoint
		strcpy(ep->hosted_interface, hosted_iface->valuestring);
	}

	if (tunnel_id == NULL) {
		ep->tunid = 0;
		print_err("Warning: Tunnel ID default is used: %ld\n",
			  ep->tunid);
	} else if (cJSON_IsString(tunnel_id)) {
		ep->tunid = atoi(tunnel_id->valuestring);
	} else {
		print_err("Error: Tunnel ID Error\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip->valuestring, &(sa.sin_addr));
		ep->ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: IP is missing or non-string\n");
		return -EINVAL;
	}

	if (eptype == NULL) {
		ep->eptype = 0;
		print_err("Warning: EP TYpe (default): %d\n", ep->eptype);
	} else if (cJSON_IsString(eptype)) {
		ep->eptype = atoi(eptype->valuestring);
	} else {
		print_err("Error: eptype ID Error\n");
		return -EINVAL;
	}

	if (mac != NULL && cJSON_IsString(mac)) {
		if (6 == sscanf(mac->valuestring,
				"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%*c", &ep->mac[0],
				&ep->mac[1], &ep->mac[2], &ep->mac[3],
				&ep->mac[4], &ep->mac[5])) {
		} else {
			/* invalid mac */
			print_err("Error: Invalid MAC\n");
			return -EINVAL;
		}
	} else {
		print_err("MAC is missing or non-string\n");
		return -EINVAL;
	}

	int i = 0;
	ep->remote_ips.remote_ips_len = 0;
	cJSON_ArrayForEach(remote_ip, remote_ips)
	{
		if (cJSON_IsString(remote_ip)) {
			struct sockaddr_in sa;
			inet_pton(AF_INET, remote_ip->valuestring,
				  &(sa.sin_addr));
			ep->remote_ips.remote_ips_val[i] = sa.sin_addr.s_addr;
			ep->remote_ips.remote_ips_len++;
		} else {
			print_err("Error: Remote IP is non-string\n");
			return -EINVAL;
		}
		i++;
		if (i == RPC_TRN_MAX_REMOTE_IPS) {
			print_err("Warning: Remote IPS reached max limited\n");
			break;
		}
	}

	return 0;
}

int trn_cli_update_ep_subcmd(CLIENT *clnt, int argc, char *argv[])
{
	ketopt_t om = KETOPT_INIT;
	struct cli_conf_data_t conf;
	cJSON *json_str = NULL;

	if (trn_cli_read_conf_str(&om, argc, argv, &conf)) {
		return -EINVAL;
	}

	char *buf = conf.conf_str;
	json_str = trn_cli_parse_json(buf);

	if (json_str == NULL) {
		return -EINVAL;
	}

	int *rc;
	rpc_trn_endpoint_t ep;
	char rpc[] = "update_ep_1";

	char veth[20];
	char hosted_itf[20];
	uint32_t remote_ips[RPC_TRN_MAX_REMOTE_IPS];
	ep.remote_ips.remote_ips_val = remote_ips;
	ep.veth = veth;
	ep.hosted_interface = hosted_itf;
	ep.interface = conf.intf;

	int err = trn_cli_parse_ep(json_str, &ep);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing endpoint config.\n");
		return -EINVAL;
	}

	rc = update_ep_1(&ep, clnt);
	if (rc == (int *)NULL) {
		print_err("RPC Error: client call failed: update_ep_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal daemon error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	dump_ep(&ep);
	print_msg(
		"update_ep_1 successfully updated endpoint %d on interface %s.\n",
		ep.ip, ep.interface);
	return 0;
}

int trn_cli_get_ep_subcmd(CLIENT *clnt, int argc, char *argv[])
{
	ketopt_t om = KETOPT_INIT;
	struct cli_conf_data_t conf;
	cJSON *json_str = NULL;

	if (trn_cli_read_conf_str(&om, argc, argv, &conf)) {
		return -EINVAL;
	}

	char *buf = conf.conf_str;
	json_str = trn_cli_parse_json(buf);

	if (json_str == NULL) {
		return -EINVAL;
	}

	rpc_trn_endpoint_key_t epkey;
	rpc_trn_endpoint_t *ep;
	char rpc[] = "get_ep_1";
	epkey.interface = conf.intf;

	int err = trn_cli_parse_ep_key(json_str, &epkey);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing endpoint config.\n");
		return -EINVAL;
	}

	ep = get_ep_1(&epkey, clnt);
	if (ep == NULL || strlen(ep->interface) == 0) {
		print_err("RPC Error: client call failed: get_ep_1.\n");
		return -EINVAL;
	}

	dump_ep(ep);
	print_msg(
		"get_ep_1 successfully queried endpoint %d on interface %s.\n",
		ep->ip, ep->interface);

	return 0;
}

int trn_cli_delete_ep_subcmd(CLIENT *clnt, int argc, char *argv[])
{
	ketopt_t om = KETOPT_INIT;
	struct cli_conf_data_t conf;
	cJSON *json_str = NULL;

	if (trn_cli_read_conf_str(&om, argc, argv, &conf)) {
		return -EINVAL;
	}

	char *buf = conf.conf_str;
	json_str = trn_cli_parse_json(buf);

	if (json_str == NULL) {
		return -EINVAL;
	}

	int *rc;
	rpc_trn_endpoint_key_t ep_key;
	char rpc[] = "delete_ep_1";
	ep_key.interface = conf.intf;

	int err = trn_cli_parse_ep_key(json_str, &ep_key);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing endpoint config.\n");
		return -EINVAL;
	}

	rc = delete_ep_1(&ep_key, clnt);
	if (rc == (int *)NULL) {
		print_err("RPC Error: client call failed: delete_ep_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal daemon error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"delete_ep_1 successfully deleted endpoint %d on interface %s.\n",
		ep_key.ip, ep_key.interface);

	return 0;
}

void dump_ep(struct rpc_trn_endpoint_t *ep)
{
	int i;

	print_msg("Interface: %s\n", ep->interface);
	print_msg("Tunnel ID: %ld\n", ep->tunid);
	print_msg("IP: 0x%x\n", ep->ip);
	print_msg("Hosted Interface: %s\n", ep->hosted_interface);
	print_msg("veth: %s\n", ep->veth);
	print_msg("EP Type: %d\n", ep->eptype);
	print_msg("Remote IPs: [");
	for (i = 0; i < ep->remote_ips.remote_ips_len; i++) {
		print_msg("0x%x", ntohl(ep->remote_ips.remote_ips_val[i]));
		if (i < ep->remote_ips.remote_ips_len - 1)
			print_msg(", ");
	}
	print_msg("]\n");
}
