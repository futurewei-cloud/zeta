// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_ftn.c
 * @author Phu Tran          (@phudtran
 *
 * @brief CLI subcommands related to ftns
 */
#include "trn_cli.h"

int trn_cli_parse_ftn(const cJSON *jsonobj, struct rpc_trn_ftn_t *ftn)
{
	cJSON *id = cJSON_GetObjectItem(jsonobj, "id");
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
		if (next_ip->valuestring[0] == '\0') {
			ftn->next_ip = 0;
		} else {
			struct sockaddr_in sa;
			inet_pton(AF_INET, next_ip->valuestring,
				  &(sa.sin_addr));
			ftn->next_ip = sa.sin_addr.s_addr;
		}
	} else {
		print_err("Error: Next IP is missing or non-string\n");
		return -EINVAL;
	}
	if (next_mac != NULL && cJSON_IsString(next_mac)) {
		if (next_mac->valuestring[0] == '\0') {
			ftn->next_mac[0] = '\0';
		} else if (6 == sscanf(next_mac->valuestring,
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

int trn_cli_update_ftn_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_ftn_t ftn;
	char rpc[] = "update_ftn_1";

	int err = trn_cli_parse_ftn(json_str, &ftn);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing ftn config.\n");
		return -EINVAL;
	}

	rc = update_ftn_1(&ftn, clnt);
	if (rc == (int *)NULL) {
		print_err("RPC Error: client call failed: update_ftn_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal daemon error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	dump_ftn(&ftn);
	print_msg("update_ftn_1 successfully updated ftn %d.\n",
		  ftn.id);
	return 0;
}

int trn_cli_get_ftn_subcmd(CLIENT *clnt, int argc, char *argv[])
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

	rpc_trn_zeta_key_t ftn_key;
	rpc_trn_ftn_t *ftn;
	char rpc[] = "get_ftn_1";

	int err = trn_cli_parse_zeta_key(json_str, &ftn_key);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing ftn config.\n");
		return -EINVAL;
	}

	ftn = get_ftn_1(&ftn_key, clnt);
	if (ftn == NULL) {
		print_err("RPC Error: client call failed: get_ftn_1.\n");
		return -EINVAL;
	}

	dump_ftn(ftn);
	print_msg("get_ftn_1 successfully queried ftn %d.\n",
		  ftn->id);

	return 0;
}

int trn_cli_delete_ftn_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_zeta_key_t ftn_key;
	char rpc[] = "delete_ftn_1";

	int err = trn_cli_parse_zeta_key(json_str, &ftn_key);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing ftn config.\n");
		return -EINVAL;
	}

	rc = delete_ftn_1(&ftn_key, clnt);
	if (rc == (int *)NULL) {
		print_err("RPC Error: client call failed: delete_ftn_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal daemon error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg("delete_ftn_1 successfully deleted ftn %d.\n",
		  ftn_key.id);

	return 0;
}

void dump_ftn(struct rpc_trn_ftn_t *ftn)
{
	print_msg("Ftn ID: %d\n", ftn->id);
	print_msg("Ftn Position: %d\n", ftn->position);
	print_msg("IP: 0x%x\n", ftn->ip);
	print_msg("MAC: %s\n", ftn->mac);
	print_msg("Next IP: 0x%x\n", ftn->ip);
	print_msg("Next MAC: %s\n", ftn->mac);
}
