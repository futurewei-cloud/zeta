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

int trn_cli_parse_zeta_key(const cJSON *jsonobj,
			   struct rpc_trn_zeta_key_t *zeta_key)
{
	cJSON *id = cJSON_GetObjectItem(jsonobj, "id");

	if (id == NULL) {
		print_err("Error: Zeta ID Error\n");
		return -EINVAL;
	} else if (cJSON_IsString(id)) {
		zeta_key->id = atoi(id->valuestring);
	} else {
		print_err("Error: ID Error\n");
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
