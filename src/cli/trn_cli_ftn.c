// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli_ftn.c
 * @author Phu Tran          (@phudtran
 *
 * @brief CLI subcommands related to ftns
 */
#include "trn_cli.h"

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

	ftn.interface = conf.intf;
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
	print_msg(
		"update_ftn_1 successfully updated endpoint %d on interface %s.\n",
		ftn.id, ftn.interface);
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
	ftn_key.interface = conf.intf;

	int err = trn_cli_parse_zeta_key(json_str, &ftn_key);
	cJSON_Delete(json_str);

	if (err != 0) {
		print_err("Error: parsing ftn config.\n");
		return -EINVAL;
	}

	ftn = get_ftn_1(&ftn_key, clnt);
	if (ftn == NULL || strlen(ftn->interface) == 0) {
		print_err("RPC Error: client call failed: get_ftn_1.\n");
		return -EINVAL;
	}

	dump_ftn(ftn);
	print_msg("get_ftn_1 successfully queried ftn %d on interface %s.\n",
		  ftn->id, ftn->interface);

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
	ftn_key.interface = conf.intf;

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

	print_msg(
		"delete_ftn_1 successfully deleted endpoint %d on interface %s.\n",
		ftn_key.id, ftn_key.interface);

	return 0;
}

void dump_ftn(struct rpc_trn_ftn_t *ftn)
{
	print_msg("Interface: %s\n", ftn->interface);
	print_msg("Ftn ID: %d\n", ftn->id);
	print_msg("Ftn Position: %d\n", ftn->position);
	print_msg("IP: 0x%x\n", ftn->ip);
	print_msg("MAC: %s\n", ftn->mac);
	print_msg("Next IP: 0x%x\n", ftn->ip);
	print_msg("Next MAC: %s\n", ftn->mac);
}
