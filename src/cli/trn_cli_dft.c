// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_dft.c
 * @author Phu Tran          (@phudtran
 *
 * @brief CLI subcommands related to dfts
 *
 */
#include "trn_cli.h"

int trn_cli_parse_dft(const cJSON *jsonobj, struct rpc_trn_dft_t *dft)
{
	cJSON *id = cJSON_GetObjectItem(jsonobj, "id");
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
		if (i == TRAN_MAX_MAGLEV_TABLE_SIZE) {
			print_err(
				"Warning: Table entries reached max limited\n");
			break;
		}
	}
	return 0;
}

int trn_cli_update_dft_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_dft_t dft;
	char rpc[] = "update_dft_1";

	uint32_t table[TRAN_MAX_MAGLEV_TABLE_SIZE];
	dft.table.table_val = table;
	dft.table.table_len = 0;
	int err = trn_cli_parse_dft(json_str, &dft);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing dft config.\n");
		return -EINVAL;
	}

	rc = update_dft_1(&dft, clnt);
	if (rc == (int *)NULL) {
		print_err("RPC Error: client call failed: update_dft_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal daemon error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	dump_dft(&dft);
	print_msg("update_dft_1 successfully updated dft %d.\n",
		  dft.id);
	return 0;
}

int trn_cli_get_dft_subcmd(CLIENT *clnt, int argc, char *argv[])
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

	rpc_trn_zeta_key_t dft_key;
	rpc_trn_dft_t *dft;
	char rpc[] = "get_dft_1";

	int err = trn_cli_parse_zeta_key(json_str, &dft_key);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing dft config.\n");
		return -EINVAL;
	}

	dft = get_dft_1(&dft_key, clnt);
	if (dft == NULL) {
		print_err("RPC Error: client call failed: get_dft_1.\n");
		return -EINVAL;
	}

	dump_dft(dft);
	print_msg("get_dft_1 successfully queried dft %d.\n",
		  dft->id);

	return 0;
}

int trn_cli_delete_dft_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_zeta_key_t dft_key;
	char rpc[] = "delete_dft_1";

	int err = trn_cli_parse_zeta_key(json_str, &dft_key);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing dft config.\n");
		return -EINVAL;
	}

	rc = delete_dft_1(&dft_key, clnt);
	if (rc == (int *)NULL) {
		print_err("RPC Error: client call failed: delete_dft_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal daemon error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg("delete_dft_1 successfully deleted dft %d.\n",
		  dft_key.id);

	return 0;
}

void dump_dft(struct rpc_trn_dft_t *dft)
{
	int i;

	print_msg("DFT ID: %d\n", dft->id);
	print_msg("Table entries: [");
	for (i = 0; i < dft->table.table_len; i++) {
		print_msg("%d", dft->table.table_val[i]);
		if (i < dft->table.table_len - 1)
			print_msg(", ");
	}
	print_msg("]\n");
}
