// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_xdp.c
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief CLI subcommands to loading/unloading manage XDP programs
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

int trn_cli_parse_ebpf_prog(const cJSON *jsonobj, rpc_trn_ebpf_prog_t *prog)
{
	cJSON *name = cJSON_GetObjectItem(jsonobj, "name");

	if (name == NULL) {
		print_err("Missing name for ebpf program to load.\n");
		return -EINVAL;
	} else if (!cJSON_IsString(name)) {
		print_err("Invalid name type, should be string\n");
		return -EINVAL;
	} else {
		const char *name_str = name->valuestring;
		if (strcmp(name_str, "xdp_tx") == 0) {
			prog->prog_idx = TRAN_TX_PROG;
		} else if (strcmp(name_str, "xdp_pass") == 0) {
			prog->prog_idx = TRAN_PASS_PROG;
		} else if (strcmp(name_str, "xdp_redirect") == 0) {
			prog->prog_idx = TRAN_REDIRECT_PROG;
		} else if (strcmp(name_str, "xdp_drop") == 0) {
			prog->prog_idx = TRAN_DROP_PROG;
		} else {
			print_err("Unsupported eBPF program %s.\n", name_str);
			return -EINVAL;
		}
	}

	if (trn_cli_parse_json_number_u32(jsonobj,
		"debug_mode", &prog->debug_mode)) {
		return -EINVAL;
	}

	return 0;
}

int trn_cli_parse_xdp(const cJSON *jsonobj, rpc_trn_xdp_intf_t *xdp_intf)
{
	int tmp;

	if (trn_cli_parse_json_string(jsonobj,
		"itf_tenant", xdp_intf->interfaces[TRAN_ITF_MAP_TENANT])) {
		return -EINVAL;
	}

	if (trn_cli_parse_json_string(jsonobj,
		"itf_zgc", xdp_intf->interfaces[TRAN_ITF_MAP_ZGC])) {
		return -EINVAL;
	}

	if (trn_cli_parse_json_number_u16n(jsonobj,
		"ibo_port", &xdp_intf->ibo_port)) {
		return -EINVAL;
	}

	cJSON *debug_mode = cJSON_GetObjectItem(jsonobj, "debug_mode");
	if (debug_mode == NULL) {
		/* Make debug_mode optional */
		xdp_intf->debug_mode = 1;
	} else {
		if (trn_cli_parse_json_number_u32(jsonobj,
			"debug_mode", &xdp_intf->debug_mode)) {
			return -EINVAL;
		}
	}

	return 0;
}

int trn_cli_unload_ebpf_prog_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_ebpf_prog_t prog;
	char rpc[] = "unload_transit_xdp_ebpf_1";

	int err = trn_cli_parse_ebpf_prog(json_str, &prog);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing ebpf program.\n");
		return -EINVAL;
	}

	rc = unload_transit_xdp_ebpf_1(&prog, clnt);
	if (rc == (int *)NULL) {
		print_err(
			"Error: call failed: unload_transit_xdp_ebpf_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"unload_transit_xdp_ebpf_1 successfully removed program %d.\n",
		prog.prog_idx);

	return 0;
}

int trn_cli_load_ebpf_prog_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_ebpf_prog_t prog;
	char rpc[] = "load_transit_xdp_ebpf_1";

	int err = trn_cli_parse_ebpf_prog(json_str, &prog);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing ebpf program config.\n");
		return -EINVAL;
	}

	rc = load_transit_xdp_ebpf_1(&prog, clnt);
	if (rc == (int *)NULL) {
		print_err(
			"Error: call failed: load_transit_xdp_ebpf_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"load_transit_xdp_ebpf_1 successfully added program %d.\n",
		prog.prog_idx);

	return 0;
}

int trn_cli_load_transit_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	char itf_tenant[TRAN_MAX_ITF_SIZE];
	char itf_zgc[TRAN_MAX_ITF_SIZE];
	rpc_trn_xdp_intf_t xdp_intf = {
		.interfaces[TRAN_ITF_MAP_TENANT] = itf_tenant,
		.interfaces[TRAN_ITF_MAP_ZGC] = itf_zgc,
	};
	char rpc[] = "load_transit_xdp_1";

	int err = trn_cli_parse_xdp(json_str, &xdp_intf);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing XDP path config.\n");
		return -EINVAL;
	}

	rc = load_transit_xdp_1(&xdp_intf, clnt);
	if (rc == (int *)NULL) {
		print_err("Error: call failed: load_transit_xdp_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"load_transit_xdp_1 successfully loaded transit xdp.\n");

	return 0;
}

int trn_cli_unload_transit_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	char itf_tenant[TRAN_MAX_ITF_SIZE];
	char itf_zgc[TRAN_MAX_ITF_SIZE];
	rpc_trn_xdp_intf_t xdp_intf = {
		.interfaces[TRAN_ITF_MAP_TENANT] = itf_tenant,
		.interfaces[TRAN_ITF_MAP_ZGC] = itf_zgc,
	};
	char rpc[] = "unload_transit_xdp_1";

	int err = trn_cli_parse_xdp(json_str, &xdp_intf);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing XDP path config.\n");
		return -EINVAL;
	}

	rc = unload_transit_xdp_1(&xdp_intf, clnt);
	if (rc == (int *)NULL) {
		print_err("Error: call failed: unload_transit_xdp_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		printf("%s fatal error, see transitd logs for details.\n", rpc);
		return -EINVAL;
	}

	printf("unload_transit_xdp_1 successfully unloaded transit xdp.\n");
	return 0;
}
