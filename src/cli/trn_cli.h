// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli.h
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief Defines methods and data structs used by the CLI frontend
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
#pragma once
#pragma GCC system_header

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <endian.h>
#include "extern/cJSON.h"
#include "extern/ketopt.h"

#include "trn_log.h"
#include "trn_rpc.h"

struct cli_conf_data_t {
	char *conf_str; // 10K conf file is too much anyway
	char intf[TRAN_MAX_ITF_SIZE]; // Interface name is 20 char including \0
};

#define print_err(f_, ...)                                                     \
	do {                                                                   \
		fprintf(stderr, f_, ##__VA_ARGS__);                            \
	} while (0)

#define print_msg(f_, ...)                                                     \
	do {                                                                   \
		printf(f_, ##__VA_ARGS__);                                     \
	} while (0)

int trn_cli_read_conf_str(ketopt_t *om, int argc, char *argv[],
			  struct cli_conf_data_t *);

cJSON *trn_cli_parse_json(const char *buf);
int trn_cli_parse_json_string(const cJSON *jsonobj, const char *const key, char *buf);
int trn_cli_parse_json_number_u16n(const cJSON *jsonobj, const char *const key,
	unsigned short *buf);
int trn_cli_parse_json_number_u32n(const cJSON *jsonobj, const char *const key,
	unsigned int *buf);
int trn_cli_parse_json_number_mac(const cJSON *jsonobj, const char *const key,
	unsigned char *buf);
int trn_cli_parse_json_number_u32(const cJSON *jsonobj, const char *const key,
	unsigned int *buf);
int trn_cli_parse_json_str_ip(const cJSON *jsonobj, const char *const key, unsigned int *buf);
int trn_cli_parse_json_str_mac(const cJSON *jsonobj, const char *const key, unsigned char *buf);
int trn_cli_parse_zeta_key(const cJSON *jsonobj,
			   struct rpc_trn_zeta_key_t *zeta_key);

int trn_cli_update_dft_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_update_chain_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_update_ftn_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_update_ep_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_delete_dft_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_delete_chain_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_delete_ftn_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_delete_ep_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_get_dft_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_get_chain_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_get_ftn_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_get_ep_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_load_transit_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_unload_transit_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_update_droplet_subcmd(CLIENT *clnt, int argc, char *argv[]);

int trn_cli_load_ebpf_prog_subcmd(CLIENT *clnt, int argc, char *argv[]);
int trn_cli_unload_ebpf_prog_subcmd(CLIENT *clnt, int argc, char *argv[]);

void dump_chain(rpc_trn_chain_t *chain);
void dump_dft(rpc_trn_dft_t *dft);
void dump_droplet(rpc_trn_droplet_t *droplet);
void dump_ep(trn_ep_t *ep);
void dump_ftn(rpc_trn_ftn_t *ftn);
