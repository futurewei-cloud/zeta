// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file trn_cli.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief Transit frontend
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

static CLIENT *clnt;

static const struct cmd {
	const char *cmd;
	int (*func)(CLIENT *clnt, int argc, char **argv);
} cmds[] = {
	{ "load-transit-xdp", trn_cli_load_transit_subcmd },
	{ "unload-transit-xdp", trn_cli_unload_transit_subcmd },
	{ "update_dft", trn_cli_update_dft_subcmd },
	{ "update_ftn", trn_cli_update_ftn_subcmd },
	{ "update-ep", trn_cli_update_ep_subcmd },
	{ "get_dft", trn_cli_get_dft_subcmd },
	{ "get_ftn", trn_cli_get_ftn_subcmd },
	{ "get-ep", trn_cli_get_ep_subcmd },
	{ "delete_dft", trn_cli_delete_dft_subcmd },
	{ "delete_ftn", trn_cli_delete_ftn_subcmd },
	{ "delete-ep", trn_cli_delete_ep_subcmd },
	{ "load-pipeline-stage", trn_cli_load_pipeline_stage_subcmd },
	{ "unload-pipeline-stage", trn_cli_unload_pipeline_stage_subcmd },
	{ 0 },
};

void cleanup(int alloced, void *ptr)
{
	if (alloced) {
		free(ptr);
		ptr = NULL;
	}
}

int main(int argc, char *argv[])
{
	ketopt_t om = KETOPT_INIT, os = KETOPT_INIT;
	int i, o;
	char LOCALHOST[] = "localhost";
	char UDP[] = "udp";
	char *server = NULL;
	char *protocol = NULL;
	int rc = 0;
	const struct cmd *c;
	int malloc_server = 0;
	int malloc_protocol = 0;

	while ((o = ketopt(&om, argc, argv, 0, "s:p:", 0)) >= 0) {
		if (o == 's') {
			server = malloc(sizeof(char) * strlen(om.arg) + 1);
			strcpy(server, om.arg);
			printf("main -s %s\n", server);
			malloc_server = 1;
		}
		if (o == 'p') {
			protocol = malloc(sizeof(char) * strlen(om.arg) + 1);
			strcpy(protocol, om.arg);
			printf("main -s %s\n", server);
			malloc_protocol = 1;
		}
	}

	if (server == NULL) {
		server = LOCALHOST;
	}

	if (protocol == NULL) {
		protocol = UDP;
	}

	printf("Connecting to %s using %s protocol.\n", server, protocol);

	clnt = clnt_create(server, RPC_TRANSIT_REMOTE_PROTOCOL,
			   RPC_TRANSIT_ALFAZERO, protocol);

	if (clnt == NULL) {
		clnt_pcreateerror(server);
		cleanup(malloc_server, server);
		cleanup(malloc_protocol, protocol);
		exit(1);
	}

	if (om.ind == argc) {
		fprintf(stderr, "missing subcommand.\n");
		cleanup(malloc_server, server);
		cleanup(malloc_protocol, protocol);
		exit(1);
	}

	for (c = cmds; c->cmd; ++c) {
		if (strcmp(argv[om.ind], c->cmd) == 0) {
			rc = (c->func(clnt, argc - om.ind, argv + om.ind));
			break;
		}
	}

	if (rc) {
		cleanup(malloc_server, server);
		cleanup(malloc_protocol, protocol);
		exit(1);
	}

	clnt_destroy(clnt);
	cleanup(malloc_server, server);
	cleanup(malloc_protocol, protocol);

	return 0;
}
