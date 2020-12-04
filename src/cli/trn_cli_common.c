// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_common.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief Defines common functions for parsing CLI input
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

cJSON *trn_cli_parse_json(const char *buf)
{
	cJSON *conf_json = cJSON_Parse(buf);
	if (conf_json == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			print_err("Error: parsing json string before: %s.\n",
				  error_ptr);
		}
		return NULL;
	}
	return conf_json;
}

int trn_cli_parse_xdp(const cJSON *jsonobj, rpc_trn_xdp_intf_t *xdp_intf)
{
	cJSON *xdp_path = cJSON_GetObjectItem(jsonobj, "xdp_path");
	cJSON *pcapfile = cJSON_GetObjectItem(jsonobj, "pcapfile");

	if (xdp_path == NULL) {
		print_err("Missing path for xdp program to load.\n");
		return -EINVAL;
	} else if (cJSON_IsString(xdp_path)) {
		strcpy(xdp_intf->xdp_path, xdp_path->valuestring);
	}

	if (pcapfile == NULL) {
		xdp_intf->pcapfile = NULL;
		print_err(
			"Warning: Missing pcapfile. Packet capture will not be available for the interface.\n");
	} else if (cJSON_IsString(pcapfile)) {
		strcpy(xdp_intf->pcapfile, pcapfile->valuestring);
	}
	return 0;
}

int trn_cli_parse_dft(const cJSON *jsonobj, struct rpc_trn_dft_t *dft)
{
	cJSON *id = cJSON_GetObjectItem(jsonobj, "id");
	cJSON *zeta_type = cJSON_GetObjectItem(jsonobj, "zeta_type");
	cJSON *entry = NULL;
	cJSON *table = cJSON_GetObjectItem(jsonobj, "table");

	if (id == NULL) {
		print_err("Error: DFT ID Error\n");
		return -EINVAL;
	} else if (cJSON_IsString(id)) {
		dft->id = atoi(id->valuestring);
	} else {
		print_err("Error: ID Error\n");
		return -EINVAL;
	}

	if (zeta_type == NULL) {
		print_err("Error: Zeta zeta_type Error\n");
		return -EINVAL;
	} else if (cJSON_IsString(zeta_type)) {
		dft->zeta_type = atoi(zeta_type->valuestring);
	} else {
		print_err("Error: Zeta Type Error\n");
		return -EINVAL;
	}

	int i = 0;
	dft->table.table_len = 0;
	cJSON_ArrayForEach(entry, table)
	{
		if (cJSON_IsString(entry)) {
			dft->table.table_val[i] = atoi(entry->valuestring);
			dft->table.table_len++;
		} else {
			print_err("Error: Table entry is non-string\n");
			return -EINVAL;
		}
		i++;
		if (i == RPC_TRN_MAX_MAGLEV_TABLE_SIZE) {
			print_err(
				"Warning: Table entries reached max limited\n");
			break;
		}
	}
	return 0;
}

int trn_cli_parse_ftn(const cJSON *jsonobj, struct rpc_trn_ftn_t *ftn)
{
	cJSON *id = cJSON_GetObjectItem(jsonobj, "id");
	cJSON *zeta_type = cJSON_GetObjectItem(jsonobj, "zeta_type");
	cJSON *position = cJSON_GetObjectItem(jsonobj, "position");
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");
	cJSON *mac = cJSON_GetObjectItem(jsonobj, "mac");
	cJSON *next_ip = cJSON_GetObjectItem(jsonobj, "next_ip");
	cJSON *next_mac = cJSON_GetObjectItem(jsonobj, "next_mac");

	if (id == NULL) {
		print_err("Error: FTN ID Error\n");
		return -EINVAL;
	} else if (cJSON_IsString(id)) {
		ftn->id = atoi(id->valuestring);
	} else {
		print_err("Error: ID Error\n");
		return -EINVAL;
	}

	if (zeta_type == NULL) {
		print_err("Error: Zeta zeta_type Error\n");
		return -EINVAL;
	} else if (cJSON_IsString(zeta_type)) {
		ftn->zeta_type = atoi(zeta_type->valuestring);
	} else {
		print_err("Error: Zeta Type Error\n");
		return -EINVAL;
	}

	if (position == NULL) {
		print_err("Error: FTN Position Error\n");
		return -EINVAL;
	} else if (cJSON_IsString(id)) {
		ftn->position = atoi(position->valuestring);
	} else {
		print_err("Error: Position Error\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip->valuestring, &(sa.sin_addr));
		ftn->ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: IP is missing or non-string\n");
		return -EINVAL;
	}
	if (mac != NULL && cJSON_IsString(mac)) {
		if (6 == sscanf(mac->valuestring,
				"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%*c",
				&ftn->mac[0], &ftn->mac[1], &ftn->mac[2],
				&ftn->mac[3], &ftn->mac[4], &ftn->mac[5])) {
		} else {
			print_err("Error: Invalid MAC\n");
			return -EINVAL;
		}
	} else {
		print_err("MAC is missing or non-string\n");
		return -EINVAL;
	}

	if (next_ip != NULL && cJSON_IsString(next_ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, next_ip->valuestring, &(sa.sin_addr));
		ftn->next_ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: Next IP is missing or non-string\n");
		return -EINVAL;
	}
	if (next_mac != NULL && cJSON_IsString(next_mac)) {
		if (6 == sscanf(next_mac->valuestring,
				"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%*c",
				&ftn->next_mac[0], &ftn->next_mac[1],
				&ftn->next_mac[2], &ftn->next_mac[3],
				&ftn->next_mac[4], &ftn->next_mac[5])) {
		} else {
			print_err("Error: Invalid Next MAC\n");
			return -EINVAL;
		}
	} else {
		print_err("Next MAC is missing or non-string\n");
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

int trn_cli_parse_zeta_key(const cJSON *jsonobj,
			   struct rpc_trn_zeta_key_t *zeta_key)
{
	cJSON *id = cJSON_GetObjectItem(jsonobj, "id");
	cJSON *zeta_type = cJSON_GetObjectItem(jsonobj, "zeta_type");

	if (id == NULL) {
		print_err("Error: Zeta ID Error\n");
		return -EINVAL;
	} else if (cJSON_IsString(id)) {
		zeta_key->id = atoi(id->valuestring);
	} else {
		print_err("Error: ID Error\n");
		return -EINVAL;
	}

	if (zeta_type == NULL) {
		print_err("Error: Zeta zeta_type Error\n");
		return -EINVAL;
	} else if (cJSON_IsString(zeta_type)) {
		zeta_key->zeta_type = atoi(zeta_type->valuestring);
	} else {
		print_err("Error: Zeta Type Error\n");
		return -EINVAL;
	}
	return 0;
}

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

int trn_cli_parse_tun_intf(const cJSON *jsonobj, rpc_trn_tun_intf_t *itf)
{
	cJSON *ip = cJSON_GetObjectItem(jsonobj, "ip");
	cJSON *mac = cJSON_GetObjectItem(jsonobj, "mac");
	cJSON *iface = cJSON_GetObjectItem(jsonobj, "iface");

	if (iface == NULL) {
		itf->interface = NULL;
		print_err("Error: Missing hosted interface, using default.\n");
		return -EINVAL;
	} else if (cJSON_IsString(iface)) {
		strcpy(itf->interface, iface->valuestring);
	}

	if (mac != NULL && cJSON_IsString(mac)) {
		if (6 == sscanf(mac->valuestring,
				"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%*c",
				&itf->mac[0], &itf->mac[1], &itf->mac[2],
				&itf->mac[3], &itf->mac[4], &itf->mac[5])) {
		} else {
			/* invalid mac */
			print_err("Error: Invalid MAC\n");
			return -EINVAL;
		}
	} else {
		print_err("Error: MAC is missing or non-string\n");
		return -EINVAL;
	}

	if (ip != NULL && cJSON_IsString(ip)) {
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip->valuestring, &(sa.sin_addr));
		itf->ip = sa.sin_addr.s_addr;
	} else {
		print_err("Error: IP is missing or non-string\n");
		return -EINVAL;
	}
	return 0;
}

int trn_cli_read_conf_str(ketopt_t *om, int argc, char *argv[],
			  struct cli_conf_data_t *conf)
{
	int i, c;
	if (conf == NULL) {
		return -EINVAL;
	}

	char conf_file[4096] = "";

	while ((c = ketopt(om, argc, argv, 0, "f:j:i:", 0)) >= 0) {
		if (c == 'j') {
			strcpy(conf->conf_str, om->arg);
		} else if (c == 'f') {
			strcpy(conf_file, om->arg);
		} else if (c == 'i') {
			strcpy(conf->intf, om->arg);
		}
	}

	if (conf->intf[0] == 0) {
		fprintf(stderr, "Missing interface.\n");
		return -EINVAL;
	}

	if (conf->conf_str[0] == 0 && conf_file[0] == 0) {
		fprintf(stderr,
			"Missing configuration string or configuration file.\n");
		return -EINVAL;
	}

	if (conf->conf_str[0] && conf_file[0]) {
		fprintf(stderr,
			"Either configuration string or configuration file is expected. Providing both is ambiguous.\n");
		return -EINVAL;
	}

	if (conf_file[0]) {
		printf("Reading Configuration file: %s\n", conf_file);

		FILE *f = fopen(conf_file, "rb");
		fseek(f, 0, SEEK_END);
		size_t fsize = ftell(f);
		size_t size = fsize + 1;
		fseek(f, 0, SEEK_SET);

		if (fsize > 4096 ||
		    fread(conf->conf_str, 1, fsize, f) < fsize) {
			fprintf(stderr,
				"Configuration file partially loaded.\n");
			exit(1);
		}
		fclose(f);

		conf->conf_str[fsize] = 0;
	}
	return 0;
}

int trn_cli_parse_ebpf_prog(const cJSON *jsonobj, rpc_trn_ebpf_prog_t *prog)
{
	cJSON *xdp_path = cJSON_GetObjectItem(jsonobj, "xdp_path");
	cJSON *stage = cJSON_GetObjectItem(jsonobj, "stage");

	if (xdp_path == NULL) {
		print_err("Missing path for xdp program to load.\n");
		return -EINVAL;
	} else if (cJSON_IsString(xdp_path)) {
		strcpy(prog->xdp_path, xdp_path->valuestring);
	}

	if (stage == NULL) {
		print_err("Error missing pipeline stage.\n");
		return -EINVAL;
	} else if (cJSON_IsString(stage)) {
		const char *stage_str = stage->valuestring;

		if (strcmp(stage_str, "ON_XDP_TX") == 0) {
			prog->stage = ON_XDP_TX;
		} else if (strcmp(stage_str, "ON_XDP_PASS") == 0) {
			prog->stage = ON_XDP_PASS;
		} else if (strcmp(stage_str, "ON_XDP_REDIRECT") == 0) {
			prog->stage = ON_XDP_REDIRECT;
		} else if (strcmp(stage_str, "ON_XDP_DROP") == 0) {
			prog->stage = ON_XDP_DROP;
		} else if (strcmp(stage_str, "ON_XDP_SCALED_EP") == 0) {
			prog->stage = ON_XDP_SCALED_EP;
		} else {
			print_err("Unsupported pipeline stage %s.\n",
				  stage_str);
			return -EINVAL;
		}
	}
	return 0;
}

int trn_cli_parse_ebpf_prog_stage(const cJSON *jsonobj,
				  rpc_trn_ebpf_prog_stage_t *prog_stage)
{
	cJSON *stage = cJSON_GetObjectItem(jsonobj, "stage");

	if (stage == NULL) {
		print_err("Error missing pipeline stage.\n");
		return -EINVAL;
	} else if (cJSON_IsString(stage)) {
		const char *stage_str = stage->valuestring;

		if (strcmp(stage_str, "ON_XDP_TX") == 0) {
			prog_stage->stage = ON_XDP_TX;
		} else if (strcmp(stage_str, "ON_XDP_PASS") == 0) {
			prog_stage->stage = ON_XDP_PASS;
		} else if (strcmp(stage_str, "ON_XDP_REDIRECT") == 0) {
			prog_stage->stage = ON_XDP_REDIRECT;
		} else if (strcmp(stage_str, "ON_XDP_DROP") == 0) {
			prog_stage->stage = ON_XDP_DROP;
		} else if (strcmp(stage_str, "ON_XDP_SCALED_EP") == 0) {
			prog_stage->stage = ON_XDP_SCALED_EP;
		} else {
			print_err("Unsupported pipeline stage %s.\n",
				  stage_str);
			return -EINVAL;
		}
	}
	return 0;
}
