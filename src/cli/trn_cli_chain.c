// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_chain.c
 * @author Phu Tran          (@phudtran
 *
 * @brief CLI subcommands related to chains
 */
#include "trn_cli.h"

int trn_cli_parse_chain(const cJSON *jsonobj, struct rpc_trn_chain_t *chain)
{
	cJSON *id = cJSON_GetObjectItem(jsonobj, "id");
	cJSON *tail_ftn = cJSON_GetObjectItem(jsonobj, "tail_ftn");

	if (id == NULL) {
		print_err("Error: Chain ID Error\n");
		return -EINVAL;
	} else if (cJSON_IsString(id)) {
		chain->id = atoi(id->valuestring);
	} else {
		print_err("Error: ID Error\n");
		return -EINVAL;
	}

	if (tail_ftn == NULL) {
		print_err("Error: Chain Tail FTN Error\n");
		return -EINVAL;
	} else if (cJSON_IsString(id)) {
		chain->tail_ftn = atoi(tail_ftn->valuestring);
	} else {
		print_err("Error: Tail FTN Error\n");
		return -EINVAL;
	}
	return 0;
}

int trn_cli_update_chain_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_chain_t chain;
	char rpc[] = "update_chain_1";

	int err = trn_cli_parse_chain(json_str, &chain);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing chain config.\n");
		return -EINVAL;
	}

	rc = update_chain_1(&chain, clnt);
	if (rc == (int *)NULL) {
		print_err("RPC Error: client call failed: update_chain_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal daemon error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	dump_chain(&chain);
	print_msg(
		"update_chain_1 successfully updated chain %d.\n",
		chain.id);
	return 0;
}

int trn_cli_get_chain_subcmd(CLIENT *clnt, int argc, char *argv[])
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

	rpc_trn_zeta_key_t chain_key;
	rpc_trn_chain_t *chain;
	char rpc[] = "get_chain_1";

	int err = trn_cli_parse_zeta_key(json_str, &chain_key);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing chain config.\n");
		return -EINVAL;
	}

	chain = get_chain_1(&chain_key, clnt);
	if (chain == NULL) {
		print_err("RPC Error: client call failed: get_chain_1.\n");
		return -EINVAL;
	}

	dump_chain(chain);
	print_msg(
		"get_chain_1 successfully queried chain %d.\n",
		chain->id);

	return 0;
}

int trn_cli_delete_chain_subcmd(CLIENT *clnt, int argc, char *argv[])
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
	rpc_trn_zeta_key_t chain_key;
	char rpc[] = "delete_chain_1";

	int err = trn_cli_parse_zeta_key(json_str, &chain_key);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing chain config.\n");
		return -EINVAL;
	}

	rc = delete_chain_1(&chain_key, clnt);
	if (rc == (int *)NULL) {
		print_err("RPC Error: client call failed: delete_chain_1.\n");
		return -EINVAL;
	}

	if (*rc != 0) {
		print_err(
			"Error: %s fatal daemon error, see transitd logs for details.\n",
			rpc);
		return -EINVAL;
	}

	print_msg(
		"delete_chain_1 successfully deleted chain %d.\n",
		chain_key.id);

	return 0;
}

void dump_chain(struct rpc_trn_chain_t *chain)
{
	print_msg("Chain ID: %d\n", chain->id);
	print_msg("Chain Tail FTN: %d\n", chain->tail_ftn);
}
