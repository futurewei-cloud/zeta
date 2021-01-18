// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_droplet.c
 * @author Bin Liang (@liangbin)
 *
 * @brief CLI subcommands related to droplets
 *
 * @copyright Copyright (c) 2019 The Authors.
 *
 */
#include "trn_cli.h"

/* Parse cJSON into struct */
int trn_cli_parse_droplet(const cJSON *jsonobj, struct rpc_trn_droplet_t *droplet)
{
	cJSON *entrances = cJSON_GetObjectItem(jsonobj, "entrances");

	if (trn_cli_parse_json_string(jsonobj, "interface", droplet->interface)) {
		return -EINVAL;
	}

	if (trn_cli_parse_json_number_u32(jsonobj, "num_entrances", &droplet->num_entrances)) {
		return -EINVAL;
	} else if ( droplet->num_entrances > TRAN_MAX_ZGC_ENTRANCES) {
		print_err("Error: num_entrances over limit %d\n", TRAN_MAX_ZGC_ENTRANCES);
		return -EINVAL;
	}

	if (entrances == NULL) {
		print_err("Error: Missing entrances\n");
		return -EINVAL;
	} else if (!cJSON_IsArray(entrances)) {
		print_err("Error: entrances should be array type\n");
		return -EINVAL;
	} else if (cJSON_GetArraySize(entrances) != droplet->num_entrances){
		print_err("Error: entrances array size not match\n");
		return -EINVAL;
	}

	int i = 0;
	cJSON *entrance;
	cJSON_ArrayForEach(entrance, entrances)	{
		if (trn_cli_parse_json_str_ip(entrance, "ip", &droplet->entrances[i].ip)) {
			return -EINVAL;
		}

		if (trn_cli_parse_json_str_mac(entrance, "mac", &droplet->entrances[i].mac[0])) {
			return -EINVAL;
		}
		i++;
	}

	return 0;
}

int trn_cli_update_droplet_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	char itf_name[TRAN_MAX_ITF_SIZE];
	rpc_trn_droplet_t droplet = {.interface = itf_name};
	char rpc[] = "update_droplet_1";

	int err = trn_cli_parse_droplet(json_str, &droplet);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing droplet config.\n");
		return -EINVAL;
	}

	rc = update_droplet_1(&droplet, clnt);
	if (rc == (int *)NULL) {
		print_err("RPC Error: client call failed: update_droplet_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal daemon error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	dump_droplet(&droplet);
	print_msg("update_droplet_1 successfully updated droplet.\n");
	return 0;
}

void dump_droplet(struct rpc_trn_droplet_t *droplet)
{
	int i;

	print_msg("Interface: %s\n", droplet->interface);
	print_msg("Num of entrances: %d\n", droplet->num_entrances);
	print_msg("Entrances: [");
	for (i = 0; i < droplet->num_entrances; i++) {
		print_msg("ip: 0x%08x mac: %02x:%02x:%02x:%02x:%02x:%02x",
			droplet->entrances[i].ip,
			droplet->entrances[i].mac[0],
			droplet->entrances[i].mac[1],
			droplet->entrances[i].mac[2],
			droplet->entrances[i].mac[3],
			droplet->entrances[i].mac[4],
			droplet->entrances[i].mac[5]);
	}
	print_msg("]\n");
}
