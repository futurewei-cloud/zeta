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
	{ "update-droplet", trn_cli_update_droplet_subcmd },
	{ "update-dft", trn_cli_update_dft_subcmd },
	{ "update-chain", trn_cli_update_chain_subcmd },
	{ "update-ftn", trn_cli_update_ftn_subcmd },
	{ "update-ep", trn_cli_update_ep_subcmd },
	{ "get-dft", trn_cli_get_dft_subcmd },
	{ "get-chain", trn_cli_get_chain_subcmd },
	{ "get-ftn", trn_cli_get_ftn_subcmd },
	{ "get-ep", trn_cli_get_ep_subcmd },
	{ "delete-dft", trn_cli_delete_dft_subcmd },
	{ "delete-chain", trn_cli_delete_chain_subcmd },
	{ "delete-ftn", trn_cli_delete_ftn_subcmd },
	{ "delete-ep", trn_cli_delete_ep_subcmd },
	{ "load-ebpf-prog", trn_cli_load_ebpf_prog_subcmd },
	{ "unload-ebpf-prog", trn_cli_unload_ebpf_prog_subcmd },
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
		if (!malloc_server && o == 's') {
			server = malloc(sizeof(char) * strlen(om.arg) + 1);
			strcpy(server, om.arg);
			malloc_server = 1;
		}
		if (!malloc_protocol && o == 'p') {
			protocol = malloc(sizeof(char) * strlen(om.arg) + 1);
			strcpy(protocol, om.arg);
			malloc_protocol = 1;
		}
	}

	if (server == NULL) {
		server = LOCALHOST;
	}

	if (protocol == NULL) {
		protocol = UDP;
	}

	if (om.ind == argc) {
		fprintf(stderr, "missing subcommand.\n");
		rc = 1;
		goto cleanup;
	}

	for (c = cmds; c->cmd; ++c) {
		if (strcmp(argv[om.ind], c->cmd) == 0) {
			break;
		}
	}

	if (!c->cmd) {
		fprintf(stderr, "invalidate subcommand %s.\n", argv[om.ind]);
		rc = 1;
		goto cleanup;
	}

	printf("RPC client connecting to %s using %s protocol for cmd %s.\n",
		server, protocol, c->cmd);

	clnt = clnt_create(server, RPC_TRANSIT_REMOTE_PROTOCOL,
			   RPC_TRANSIT_ALFAZERO, protocol);

	if (clnt == NULL) {
		clnt_pcreateerror(server);
		rc = 1;
		goto cleanup;
	}

#ifdef DEBUG_TIMEOUT
	struct timeval tv;
	tv.tv_sec = 60; /* change timeout to 1 minute */
	tv.tv_usec = 0;
	clnt_control(clnt, CLSET_TIMEOUT, &tv);
#endif

	rc = (c->func(clnt, argc - om.ind, argv + om.ind));
	if (rc) {
		fprintf(stderr, "RPC subcommand failed, rc %d.\n", rc);
		rc = 1;
		goto cleanup;
	}

cleanup:
	if (malloc_server) {
		free(server);
	}
	if (malloc_protocol) {
		free(protocol);
	}
	if (clnt) {
		clnt_destroy(clnt);
	}
	return rc;
}
