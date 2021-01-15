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

int trn_cli_parse_json_string(const cJSON *jsonobj, const char *const key, char *buf)
{
	cJSON *item = cJSON_GetObjectItem(jsonobj, key);

	if (item == NULL) {
		print_err("Missing %s\n", key);
		return -EINVAL;
	} else if (!cJSON_IsString(item)) {
		print_err("Invalid %s type, should be string\n", key);
		return -EINVAL;
	}
	strcpy(buf, item->valuestring);
	return 0;
}

int trn_cli_parse_json_number_u16n(const cJSON *jsonobj, const char *const key,
	unsigned short *buf)
{
	cJSON *item = cJSON_GetObjectItem(jsonobj, key);

	if (item == NULL) {
		print_err("Missing %s\n", key);
		return -EINVAL;
	} else if (!cJSON_IsNumber(item)) {
		print_err("Invalid %s type, should be number\n", key);
		return -EINVAL;
	}
	unsigned short tmp = (unsigned short)item->valuedouble;
	*buf = 0;
	for (unsigned int i = 0; i < sizeof(unsigned short); i++) {
		*buf = (*buf << 8) | (tmp & 0xFF);
		tmp >>= 8;
	}
	return 0;
}

int trn_cli_parse_json_number_u32n(const cJSON *jsonobj, const char *const key,
	unsigned int *buf)
{
	cJSON *item = cJSON_GetObjectItem(jsonobj, key);

	if (item == NULL) {
		print_err("Missing %s\n", key);
		return -EINVAL;
	} else if (!cJSON_IsNumber(item)) {
		print_err("Invalid %s type, should be number\n", key);
		return -EINVAL;
	}
	unsigned int tmp = (unsigned int)item->valuedouble;
	*buf = 0;
	for (unsigned int i = 0; i < sizeof(unsigned int); i++) {
		*buf = (*buf << 8) | (tmp & 0xFF);
		tmp >>= 8;
	}
	return 0;
}

int trn_cli_parse_json_number_mac(const cJSON *jsonobj, const char *const key,
	unsigned char *buf)
{
	cJSON *item = cJSON_GetObjectItem(jsonobj, key);

	if (item == NULL) {
		print_err("Missing %s\n", key);
		return -EINVAL;
	} else if (!cJSON_IsNumber(item)) {
		print_err("Invalid %s type, should be number\n", key);
		return -EINVAL;
	}
	uint64_t vd = ((uint64_t)item->valuedouble & 0xFFFFFFFFFFFF);
	for (int i = 0; i < 6; i++) {
		buf[5 - i] = (unsigned char)vd;
		vd >>= 8;
	}
	return 0;
}

int trn_cli_parse_json_number_u32(const cJSON *jsonobj, const char *const key,
	unsigned int *buf)
{
	cJSON *item = cJSON_GetObjectItem(jsonobj, key);

	if (item == NULL) {
		print_err("Missing %s\n", key);
		return -EINVAL;
	} else if (!cJSON_IsNumber(item)) {
		print_err("Invalid %s type, should be number\n", key);
		return -EINVAL;
	}
	*buf = (unsigned int)item->valuedouble;
	return 0;
}

int trn_cli_parse_json_str_ip(const cJSON *jsonobj, const char *const key, unsigned int *buf)
{
	cJSON *item = cJSON_GetObjectItem(jsonobj, key);

	if (item == NULL) {
		print_err("Missing %s\n", key);
		return -EINVAL;
	} else if (!cJSON_IsString(item)) {
		print_err("Invalid ip type, should be string\n");
		return -EINVAL;
	} else if (inet_pton(AF_INET, item->valuestring, buf) <= 0) {
		print_err("Failed to convert ip %s", item->valuestring);
		return -EINVAL;
	}
	return 0;
}

int trn_cli_parse_json_str_mac(const cJSON *jsonobj, const char *const key, unsigned char *buf)
{
	cJSON *item = cJSON_GetObjectItem(jsonobj, key);

	if (item == NULL) {
		print_err("Missing %s\n", key);
		return -EINVAL;
	} else if (!cJSON_IsString(item)) {
		print_err("Invalid mac type, should be string\n");
		return -EINVAL;
	} else if (6 != sscanf(item->valuestring,
				"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%*c",
				buf, buf+1, buf+2, buf+3, buf+4, buf+5)) {
		print_err("Invalid mac %s", item->valuestring);
		return -EINVAL;
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

int trn_cli_read_conf_str(ketopt_t *om, int argc, char *argv[],
			  struct cli_conf_data_t *conf)
{
	int i, c;
	char conf_file[TRAN_MAX_PATH_SIZE];
	if (conf == NULL) {
		return -EINVAL;
	}

	conf->conf_str = NULL;
	conf->intf[0] = 0;
	conf_file[0] = 0;

	while ((c = ketopt(om, argc, argv, 0, "f:j:", 0)) >= 0) {
		if (c == 'j') {
			if (strlen(om->arg) >= TRAN_MAX_CLI_JSON_STR) {
				print_msg("Mega Json string %lu\n", strlen(om->arg));
			}
			conf->conf_str = om->arg;
		} else if (c == 'f') {
			if (strlen(om->arg) >= TRAN_MAX_PATH_SIZE) {
				print_err("file name over limit %lu/%d\n",
					strlen(om->arg), TRAN_MAX_PATH_SIZE);
				return -EINVAL;
			}
			strcpy(conf_file, om->arg);
		}
	}

	if (!conf->conf_str && conf_file[0] == 0) {
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

		if (fsize >= TRAN_MAX_CLI_JSON_STR ||
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
