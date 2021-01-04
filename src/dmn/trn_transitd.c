// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @author Sherif Abdelwahab (@zasherif)
 *
 * @brief Transit daemon
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

#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <stdint.h>
#include <sys/mount.h>
#include <signal.h>
#include <pthread.h>

#include "trn_transitd.h"

#define TRANSITLOGNAME "transit"

void rpc_transit_remote_protocol_1(struct svc_req *rqstp,
				   register SVCXPRT *transp);

void sighandler(int signo)
{
	TRN_LOG_INFO("Received signal: %d", signo);

	switch (signo) {
	default:
		TRN_LOG_INFO("Exit!.");
	};

	TRN_LOG_CLOSE();
	exit(1);
}

/* thread entrance for rpc server */
void *entrance_rpc(void *arg)
{
	UNUSED(arg);

	/* (re)Mount the bpf fs*/
	umount("/sys/fs/bpf");
	if (mount("bpf", "/sys/fs/bpf", "bpf", 0, "mode=0700")) {
		TRN_LOG_ERROR("mount -t bpf bpf /sys/fs/bpf failed: %s\n",
			      strerror(errno));
		exit(1);
	}

	register SVCXPRT *transp;
	pmap_unset(RPC_TRANSIT_REMOTE_PROTOCOL, RPC_TRANSIT_ALFAZERO);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		TRN_LOG_ERROR("cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, RPC_TRANSIT_REMOTE_PROTOCOL,
			  RPC_TRANSIT_ALFAZERO, rpc_transit_remote_protocol_1,
			  IPPROTO_UDP)) {
		TRN_LOG_ERROR(
			"unable to register (RPC_TRANSIT_REMOTE_PROTOCOL, RPC_TRANSIT_ALFAZERO, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		TRN_LOG_ERROR("cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, RPC_TRANSIT_REMOTE_PROTOCOL,
			  RPC_TRANSIT_ALFAZERO, rpc_transit_remote_protocol_1,
			  IPPROTO_TCP)) {
		TRN_LOG_ERROR(
			"unable to register (RPC_TRANSIT_REMOTE_PROTOCOL, RPC_TRANSIT_ALFAZERO, tcp).");
		exit(1);
	}

	TRN_LOG_INFO(
		"Press ctrl-c, or send SIGTERM to process ID %d, to gracefully exit program.",
		getpid());

	TRN_LOG_INFO("RPC handler thread running");
	svc_run();
	TRN_LOG_ERROR("svc_run returned");

	pthread_exit(NULL);
}

/* thread entrance for datapath assistant */
void *entrance_dpa(void *arg) {
	UNUSED(arg);

	TRN_LOG_INFO("DPA thread running");
	trn_transit_dp_assistant();
	TRN_LOG_ERROR("DPA thread ending");
	pthread_exit(NULL);
}

int main()
{
	struct sigaction act;
	pthread_t thr_rpc, thr_dpa;
	int rc;

	TRN_LOG_INIT(TRANSITLOGNAME);

	/* Create a signal handler shutdown gracefully */
	memset(&act, 0, sizeof(act));
	act.sa_handler = sighandler;

	sigaction(SIGINT, &act, 0);
	sigaction(SIGTERM, &act, 0);

	if ((rc = pthread_create(&thr_rpc, NULL, entrance_rpc, NULL))) {
		TRN_LOG_ERROR("cannot create rpc handler thread, rc: %d", rc);
		printf("cannot create rpc handler thread, rc: %d\n", rc);
		exit(1);
    }

	if ((rc = pthread_create(&thr_dpa, NULL, entrance_dpa, NULL))) {
		TRN_LOG_ERROR("cannot create datapath assistant thread, rc: %d", rc);
		printf("cannot create datapath assistant thread, rc: %d\n", rc);
		exit(1);
    }

	pthread_join(thr_rpc, NULL);
	pthread_join(thr_dpa, NULL);

	TRN_LOG_ERROR("All transitd threads ended");

	TRN_LOG_CLOSE();

	exit(1);
	/* NOTREACHED */
}
