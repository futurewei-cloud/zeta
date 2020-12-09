// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file test_cli.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief cli unit tests
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
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "cli/trn_cli.h"
#include "trn_log.h"
#include <stdio.h>
#include <stdbool.h>
#include <sys/resource.h>

static int clnt_perror_called = 0;

int __wrap_setrlimit(int resource, const struct rlimit *rlim)
{
	UNUSED(resource);
	UNUSED(rlim);
	return 0;
}

int *__wrap_update_dft_1(rpc_trn_dft_t *dft, CLIENT *clnt)
{
	check_expected_ptr(dft);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_chain_1(rpc_trn_chain_t *chain, CLIENT *clnt)
{
	check_expected_ptr(chain);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_ftn_1(rpc_trn_ftn_t *ftn, CLIENT *clnt)
{
	check_expected_ptr(ftn);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_update_ep_1(rpc_trn_endpoint_t *ep, CLIENT *clnt)
{
	check_expected_ptr(ep);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_load_transit_xdp_1(rpc_trn_xdp_intf_t *itf, CLIENT *clnt)
{
	UNUSED(itf);
	UNUSED(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_unload_transit_xdp_1(rpc_trn_xdp_intf_t *itf, CLIENT *clnt)
{
	UNUSED(itf);
	UNUSED(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

rpc_trn_dft_t *__wrap_get_dft_1(rpc_trn_zeta_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	rpc_trn_dft_t *retval = mock_ptr_type(rpc_trn_dft_t *);
	function_called();
	return retval;
}

rpc_trn_chain_t *__wrap_get_chain_1(rpc_trn_zeta_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	rpc_trn_chain_t *retval = mock_ptr_type(rpc_trn_chain_t *);
	function_called();
	return retval;
}

rpc_trn_ftn_t *__wrap_get_ftn_1(rpc_trn_zeta_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	rpc_trn_ftn_t *retval = mock_ptr_type(rpc_trn_ftn_t *);
	function_called();
	return retval;
}

rpc_trn_endpoint_t *__wrap_get_ep_1(rpc_trn_endpoint_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	rpc_trn_endpoint_t *retval = mock_ptr_type(rpc_trn_endpoint_t *);
	function_called();
	return retval;
}
int *__wrap_delete_dft_1(rpc_trn_zeta_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_chain_1(rpc_trn_zeta_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_ftn_1(rpc_trn_zeta_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_delete_ep_1(rpc_trn_endpoint_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

static inline int cmpfunc(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

static int check_ep_equal(const LargestIntegralType value,
			  const LargestIntegralType check_value_data)
{
	struct rpc_trn_endpoint_t *ep = (struct rpc_trn_endpoint_t *)value;
	struct rpc_trn_endpoint_t *c_ep =
		(struct rpc_trn_endpoint_t *)check_value_data;
	int i;

	assert_string_equal(ep->interface, c_ep->interface);

	assert_string_equal(ep->hosted_interface, c_ep->hosted_interface);

	assert_string_equal(ep->veth, c_ep->veth);

	assert_memory_equal(ep->mac, c_ep->mac, sizeof(char) * 6);

	assert_int_equal(ep->ip, c_ep->ip);

	assert_int_equal(ep->eptype, c_ep->eptype);

	assert_int_equal(ep->tunid, c_ep->tunid);

	assert_int_equal(ep->remote_ips.remote_ips_len,
			 c_ep->remote_ips.remote_ips_len);

	if (ep->remote_ips.remote_ips_len == 0) {
		return true;
	}

	qsort(ep->remote_ips.remote_ips_val, ep->remote_ips.remote_ips_len,
	      sizeof(uint32_t), cmpfunc);

	qsort(c_ep->remote_ips.remote_ips_val, c_ep->remote_ips.remote_ips_len,
	      sizeof(uint32_t), cmpfunc);

	for (i = 0; i < ep->remote_ips.remote_ips_len; i++) {
		if (c_ep->remote_ips.remote_ips_val[i] !=
		    ep->remote_ips.remote_ips_val[i]) {
			return false;
		}
	}

	return true;
}

static int check_ep_key_equal(const LargestIntegralType value,
			      const LargestIntegralType check_value_data)
{
	struct rpc_trn_endpoint_key_t *ep_key =
		(struct rpc_trn_endpoint_key_t *)value;
	struct rpc_trn_endpoint_key_t *c_ep_key =
		(struct rpc_trn_endpoint_key_t *)check_value_data;

	assert_string_equal(ep_key->interface, c_ep_key->interface);

	assert_int_equal(ep_key->tunid, c_ep_key->tunid);

	assert_int_equal(ep_key->ip, c_ep_key->ip);

	return true;
}

static int check_dft_equal(const LargestIntegralType value,
			   const LargestIntegralType check_value_data)
{
	struct rpc_trn_dft_t *dft = (struct rpc_trn_dft_t *)value;
	struct rpc_trn_dft_t *c_dft = (struct rpc_trn_dft_t *)check_value_data;
	int i;

	assert_string_equal(dft->interface, c_dft->interface);
	assert_int_equal(dft->id, c_dft->id);

	qsort(dft->table.table_val, dft->table.table_len, sizeof(uint32_t),
	      cmpfunc);

	qsort(c_dft->table.table_val, c_dft->table.table_len, sizeof(uint32_t),
	      cmpfunc);

	for (i = 0; i < dft->table.table_len; i++) {
		assert_int_equal(c_dft->table.table_val[i],
				 dft->table.table_val[i]);
	}

	return true;
}

static int check_chain_equal(const LargestIntegralType value,
			     const LargestIntegralType check_value_data)
{
	struct rpc_trn_chain_t *chain = (struct rpc_trn_chain_t *)value;
	struct rpc_trn_chain_t *c_chain =
		(struct rpc_trn_chain_t *)check_value_data;

	assert_string_equal(chain->interface, c_chain->interface);
	assert_int_equal(chain->id, c_chain->id);
	assert_int_equal(chain->tail_ftn, c_chain->tail_ftn);

	return true;
}

static int check_ftn_equal(const LargestIntegralType value,
			   const LargestIntegralType check_value_data)
{
	struct rpc_trn_ftn_t *ftn = (struct rpc_trn_ftn_t *)value;
	struct rpc_trn_ftn_t *c_ftn = (struct rpc_trn_ftn_t *)check_value_data;

	assert_string_equal(ftn->interface, c_ftn->interface);
	assert_int_equal(ftn->id, c_ftn->id);
	assert_int_equal(ftn->position, c_ftn->position);
	assert_int_equal(ftn->ip, c_ftn->ip);
	assert_int_equal(ftn->next_ip, c_ftn->next_ip);
	assert_string_equal(ftn->mac, c_ftn->mac);
	assert_string_equal(ftn->next_mac, c_ftn->next_mac);

	return true;
}

static int check_zeta_key_equal(const LargestIntegralType value,
				const LargestIntegralType check_value_data)
{
	struct rpc_trn_zeta_key_t *zeta_key =
		(struct rpc_trn_zeta_key_t *)value;
	struct rpc_trn_zeta_key_t *c_zeta_key =
		(struct rpc_trn_zeta_key_t *)check_value_data;

	assert_string_equal(zeta_key->interface, c_zeta_key->interface);
	assert_int_equal(zeta_key->id, c_zeta_key->id);

	return true;
}

static void test_trn_cli_update_dft_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_dft_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-dft", "-i", "eth0", "-j",
			  QUOTE({ "id": "3", "table": "[0,1,2,3,4,5]" }) };

	char *argv2[] = { "update-dft", "-i", "eth0", "-j",
			  QUOTE({ "id": 3, "table": "[0,1,2,3,4,5]" }) };

	char *argv3[] = { "update-dft", "-i", "eth0", "-j",
			  QUOTE({ "id": "3", "table": [0, 1, 2, 3, 4, 5] }) };

	char itf[] = "eth0";
	uint32_t table[] = { 0, 1, 2, 3, 4, 5 };
	uint32_t id = 3;
	struct rpc_trn_dft_t exp_dft = {
		.interface = itf,
		.id = id,
		.table = { .table_len = 6, .table_val = table },
	};

	/* Test call update_dft successfully */
	TEST_CASE("update_dft succeed with well formed dft json input");
	update_dft_1_ret_val = 0;
	expect_function_call(__wrap_update_dft_1);
	will_return(__wrap_update_dft_1, &update_dft_1_ret_val);
	expect_check(__wrap_update_dft_1, dft, check_dft_equal, &exp_dft);
	expect_any(__wrap_update_dft_1, clnt);
	rc = trn_cli_update_dft_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse dft input error*/
	TEST_CASE("update_dft is not called with non-string field");
	rc = trn_cli_update_dft_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("update_dft is not called malformed json");
	rc = trn_cli_update_dft_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_chain_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_chain_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-chain", "-i", "eth0", "-j",
			  QUOTE({ "id": "3", "tail_ftn": "1" }) };

	char *argv2[] = { "update-chain", "-i", "eth0", "-j",
			  QUOTE({ "id": 3, "tail_ftn": "1" }) };

	char *argv3[] = { "update-chain", "-i", "eth0", "-j",
			  QUOTE({ "id: 3, tail_ftn": "1" }) };

	char itf[] = "eth0";
	uint32_t id = 3;
	uint32_t tail_ftn = 1;

	struct rpc_trn_chain_t exp_chain = {
		.interface = itf,
		.id = id,
		.tail_ftn = tail_ftn,
	};

	/* Test call update_chain successfully */
	TEST_CASE("update_chain succeed with well formed chain json input");
	update_chain_1_ret_val = 0;
	expect_function_call(__wrap_update_chain_1);
	will_return(__wrap_update_chain_1, &update_chain_1_ret_val);
	expect_check(__wrap_update_chain_1, chain, check_chain_equal,
		     &exp_chain);
	expect_any(__wrap_update_chain_1, clnt);
	rc = trn_cli_update_chain_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse chain input error*/
	TEST_CASE("update_chains not called with non-string field");
	rc = trn_cli_update_chain_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("update_chain is not called malformed json");
	rc = trn_cli_update_chain_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_ftn_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_ftn_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-ftn", "-i", "eth0", "-j", QUOTE({
				  "id": "3",
				  "position": "0",
				  "ip": "10.0.0.1",
				  "mac": "1:2:3:4:5:6",
				  "next_ip": "10.0.0.2",
				  "next_mac": "7:8:9:1:2:3"
			  }) };

	char *argv2[] = { "update-ftn", "-i", "eth0", "-j", QUOTE({
				  "id": 3,
				  "position": "0",
				  "ip": "10.0.0.1",
				  "mac": "1:2:3:4:5:6",
				  "next_ip": "10.0.0.2",
				  "next_mac": "7:8:9:1:2:3"
			  }) };

	char *argv3[] = { "update-ftn", "-i", "eth0", "-j", QUOTE({
				  "id": "3",
				  "position": "0",
				  "ip": 10.0.0.1,
				  "mac": "1:2:3:4:5:6",
				  "next_ip": "10.0.0.2",
				  "next_mac": "7:8:9:1:2:3"
			  }) };

	char itf[] = "eth0";
	uint32_t id = 3;
	uint8_t position = 0;
	uint32_t ip = 0x100000a;
	uint32_t next_ip = 0x200000a;
	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char next_mac[6] = { 7, 8, 9, 1, 2, 3 };

	struct rpc_trn_ftn_t exp_ftn = {
		.interface = itf,
		.id = id,
		.position = position,
		.ip = ip,
		.next_ip = next_ip,
	};

	memcpy(exp_ftn.mac, mac, sizeof(char) * 6);
	memcpy(exp_ftn.next_mac, next_mac, sizeof(char) * 6);

	/* Test call update_ftn successfully */
	TEST_CASE("update_ftn succeed with well formed ftn json input");
	update_ftn_1_ret_val = 0;
	expect_function_call(__wrap_update_ftn_1);
	will_return(__wrap_update_ftn_1, &update_ftn_1_ret_val);
	expect_check(__wrap_update_ftn_1, ftn, check_ftn_equal, &exp_ftn);
	expect_any(__wrap_update_ftn_1, clnt);
	rc = trn_cli_update_ftn_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ftn input error*/
	TEST_CASE("update_ftn is not called with non-string field");
	rc = trn_cli_update_ftn_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("update_ftn is not called malformed json");
	rc = trn_cli_update_ftn_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_update_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int update_ep_1_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "update-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "ip": "10.0.0.1",
				  "eptype": "1",
				  "mac": "1:2:3:4:5:6",
				  "veth": "veth0",
				  "remote_ips": ["10.0.0.2"],
				  "hosted_iface": "peer"
			  }) };

	char *argv2[] = { "update-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": 3,
				  "ip": "10.0.0.1",
				  "eptype": "1",
				  "mac": "1:2:3:4:5:6",
				  "veth": "veth0",
				  "remote_ips": ["10.0.0.2"],
				  "hosted_iface": "peer"
			  }) };

	char *argv3[] = { "update-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "mac": "1:2:3:4:5:6",
				  "veth": "veth0",
				  "remote_ips": ["10.0.0.2"],
				  "hosted_iface": "peer"
			  }) };

	char *argv4[] = { "update-ep", "-i", "eth0", "-j", QUOTE({
				  "tunnel_id": "3",
				  "mac": "1:2:3:4:5:6",
				  "veth": "veth0",
				  "remote_ips": [10.0.0.2],
				  "hosted_iface": "peer"
			  }) };

	char itf[] = "eth0";
	char vitf[] = "veth0";
	char hosted_itf[] = "peer";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t exp_ep = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 1,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 3,
	};

	memcpy(exp_ep.mac, mac, sizeof(char) * 6);

	/* Test call update_ep successfully */
	TEST_CASE("update_ep succeed with well formed endpoint json input");
	update_ep_1_ret_val = 0;
	expect_function_call(__wrap_update_ep_1);
	will_return(__wrap_update_ep_1, &update_ep_1_ret_val);
	expect_check(__wrap_update_ep_1, ep, check_ep_equal, &exp_ep);
	expect_any(__wrap_update_ep_1, clnt);
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("update_ep is not called with non-string field");
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("update_ep is not called with missing required field");
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("update_ep is not called malformed json");
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_ep_1 return error*/
	TEST_CASE("update-ep subcommand fails if update_ep_1 returns error");
	update_ep_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_ep_1);
	will_return(__wrap_update_ep_1, &update_ep_1_ret_val);
	expect_any(__wrap_update_ep_1, ep);
	expect_any(__wrap_update_ep_1, clnt);
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_ep_1 return NULL*/
	TEST_CASE("update-ep subcommand fails if update_ep_1 returns NULl");
	expect_function_call(__wrap_update_ep_1);
	will_return(__wrap_update_ep_1, NULL);
	expect_any(__wrap_update_ep_1, ep);
	expect_any(__wrap_update_ep_1, clnt);
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_load_transit_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int load_transit_xdp_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "load-transit-xdp", "-i", "eth0", "-j", QUOTE({
				  "xdp_path": "/path/to/xdp/object/file",
				  "pcapfile": "/path/to/bpf/pinned/map"
			  }) };

	char *argv2[] = { "load-transit-xdp", "-i", "eth0", "-j",
			  QUOTE({ "pcapfile": "/path/to/bpf/pinned/map" }) };

	char *argv3[] = { "load-transit-xdp", "-i", "eth0", "-j",
			  QUOTE({ "xdp_path": "/path/to/xdp/object/file" }) };

	/* Test call load_transit_xdp_1 successfully */
	TEST_CASE("load_transit_xdp succeed with well formed input");
	load_transit_xdp_ret_val = 0;
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, &load_transit_xdp_ret_val);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	TEST_CASE("load_transit_xdp fails if path to object file is missing");
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("load_transit_xdp succeed even if pcap map is missing");
	load_transit_xdp_ret_val = 0;
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, &load_transit_xdp_ret_val);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, 0);

	TEST_CASE("load_transit_xdp fails if rpc returns Error");
	load_transit_xdp_ret_val = -EINVAL;
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, &load_transit_xdp_ret_val);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call load_transit_xdp return NULL*/
	TEST_CASE("load_transit_xdp subcommand fails if rpc returns NULl");
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, NULL);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_unload_transit_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;
	int unload_transit_xdp_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "load-transit-xdp", "-i", "eth0", "-j", QUOTE({}) };

	/* Test call unload_transit_xdp_1 successfully */
	TEST_CASE("unload_transit_xdp succeed with well formed (empty) input");
	unload_transit_xdp_ret_val = 0;
	expect_function_call(__wrap_unload_transit_xdp_1);
	will_return(__wrap_unload_transit_xdp_1, &unload_transit_xdp_ret_val);
	rc = trn_cli_unload_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test call unload_transit_xdp return error*/
	TEST_CASE(
		"unload_transit_xdp subcommand fails if update_net_1 returns error");
	unload_transit_xdp_ret_val = -EINVAL;
	expect_function_call(__wrap_unload_transit_xdp_1);
	will_return(__wrap_unload_transit_xdp_1, &unload_transit_xdp_ret_val);
	rc = trn_cli_unload_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call unload_transit_xdp_! return NULL*/
	TEST_CASE("unload_transit_xdp subcommand fails if rpc returns NULl");
	expect_function_call(__wrap_unload_transit_xdp_1);
	will_return(__wrap_unload_transit_xdp_1, NULL);
	rc = trn_cli_unload_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_get_dft_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";
	uint32_t table[] = { 0, 1, 2, 3, 4, 5 };
	uint32_t id = 3;
	struct rpc_trn_dft_t get_dft_1_ret_val = {
		.interface = itf,
		.id = id,
		.table = { .table_len = 6, .table_val = table },
	};

	/* Test cases */
	char *argv1[] = { "get-dft", "-i", "eth0", "-j", QUOTE({ "id": "3" }) };

	char *argv2[] = { "get-dft", "-i", "eth0", "-j", QUOTE({ "id": 3 }) };

	char *argv3[] = { "get-dft", "-i", "eth0", "-j", QUOTE({ "id 3" }) };

	struct rpc_trn_zeta_key_t exp_dft_key = {
		.interface = itf,
		.id = 3,
	};

	/* Test call get_dft successfully */
	TEST_CASE("get_dft succeed with well formed endpoint json input");
	expect_function_call(__wrap_get_dft_1);
	will_return(__wrap_get_dft_1, &get_dft_1_ret_val);
	expect_check(__wrap_get_dft_1, argp, check_zeta_key_equal,
		     &exp_dft_key);
	expect_any(__wrap_get_dft_1, clnt);
	rc = trn_cli_get_dft_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse dft input error*/
	TEST_CASE("get_dft is not called with non-string field");
	rc = trn_cli_get_dft_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("get_dft is not called with missing required field");
	rc = trn_cli_get_dft_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_dft_1 return NULL*/
	TEST_CASE("get-dft subcommand fails if get_dft_1 returns NULl");
	expect_function_call(__wrap_get_dft_1);
	will_return(__wrap_get_dft_1, NULL);
	expect_any(__wrap_get_dft_1, argp);
	expect_any(__wrap_get_dft_1, clnt);
	rc = trn_cli_get_dft_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_get_chain_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";
	uint32_t id = 3;
	uint32_t tail_ftn = 1;

	struct rpc_trn_chain_t get_chain_1_ret_val = {
		.interface = itf,
		.tail_ftn = tail_ftn,
	};

	/* Test cases */
	char *argv1[] = { "get-chain", "-i", "eth0", "-j",
			  QUOTE({ "id": "3" }) };

	char *argv2[] = { "get-chain", "-i", "eth0", "-j", QUOTE({ "id": 3 }) };

	char *argv3[] = { "get-chain", "-i", "eth0", "-j",
			  QUOTE({ "id : 3 " }) };

	struct rpc_trn_zeta_key_t exp_chain_key = {
		.interface = itf,
		.id = 3,
	};

	/* Test call get_chain successfully */
	TEST_CASE("get_chain succeed with well formed endpoint json input");
	expect_function_call(__wrap_get_chain_1);
	will_return(__wrap_get_chain_1, &get_chain_1_ret_val);
	expect_check(__wrap_get_chain_1, argp, check_zeta_key_equal,
		     &exp_chain_key);
	expect_any(__wrap_get_chain_1, clnt);
	rc = trn_cli_get_chain_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse chain input error*/
	TEST_CASE("get_chain is not called with non-string field");
	rc = trn_cli_get_chain_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("get_chain is not called with missing required field");
	rc = trn_cli_get_chain_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_chain_1 return NULL*/
	TEST_CASE("get-chain subcommand fails if get_chain_1 returns NULL");
	expect_function_call(__wrap_get_chain_1);
	will_return(__wrap_get_chain_1, NULL);
	expect_any(__wrap_get_chain_1, argp);
	expect_any(__wrap_get_chain_1, clnt);
	rc = trn_cli_get_chain_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_get_ftn_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";
	uint32_t id = 3;
	uint8_t position = 0;
	uint32_t ip = 0x100000a;
	uint32_t next_ip = 0x200000a;
	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char next_mac[6] = { 7, 8, 9, 1, 2, 3 };

	struct rpc_trn_ftn_t get_ftn_1_ret_val = {
		.interface = itf,
		.id = id,
		.position = position,
		.ip = ip,
		.next_ip = next_ip,
	};
	memcpy(get_ftn_1_ret_val.mac, mac, sizeof(char) * 6);
	memcpy(get_ftn_1_ret_val.next_mac, next_mac, sizeof(char) * 6);

	/* Test cases */
	char *argv1[] = { "get-ftn", "-i", "eth0", "-j", QUOTE({ "id": "3" }) };

	char *argv2[] = { "get-ftn", "-i", "eth0", "-j", QUOTE({ "id": 3 }) };

	char *argv3[] = { "get-ftn", "-i", "eth0", "-j", QUOTE({ "id 3" }) };

	struct rpc_trn_zeta_key_t exp_ftn_key = {
		.interface = itf,
		.id = 3,
	};

	/* Test call get_ftn successfully */
	TEST_CASE("get_ftn succeed with well formed endpoint json input");
	expect_function_call(__wrap_get_ftn_1);
	will_return(__wrap_get_ftn_1, &get_ftn_1_ret_val);
	expect_check(__wrap_get_ftn_1, argp, check_zeta_key_equal,
		     &exp_ftn_key);
	expect_any(__wrap_get_ftn_1, clnt);
	rc = trn_cli_get_ftn_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ftn input error*/
	TEST_CASE("get_ftn is not called with non-string field");
	rc = trn_cli_get_ftn_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("get_ftn is not called with missing required field");
	rc = trn_cli_get_ftn_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_ftn_1 return NULL*/
	TEST_CASE("get-ftn subcommand fails if get_ftn_1 returns NULL");
	expect_function_call(__wrap_get_ftn_1);
	will_return(__wrap_get_ftn_1, NULL);
	expect_any(__wrap_get_ftn_1, argp);
	expect_any(__wrap_get_ftn_1, clnt);
	rc = trn_cli_get_ftn_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_get_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";
	char vitf[] = "veth0";
	char hosted_itf[] = "peer";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t get_ep_1_ret_val = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 1,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 3,
	};
	memcpy(get_ep_1_ret_val.mac, mac, sizeof(char) * 6);

	/* Test cases */
	char *argv1[] = { "get-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": "10.0.0.1" }) };

	char *argv2[] = { "get-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": 3, "ip": "10.0.0.1" }) };

	char *argv3[] = { "get-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3" }) };

	char *argv4[] = { "get-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": [10.0.0.2] }) };

	struct rpc_trn_endpoint_key_t exp_ep_key = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};

	/* Test call get_ep successfully */
	TEST_CASE("get_ep succeed with well formed endpoint json input");
	expect_function_call(__wrap_get_ep_1);
	will_return(__wrap_get_ep_1, &get_ep_1_ret_val);
	expect_check(__wrap_get_ep_1, argp, check_ep_key_equal, &exp_ep_key);
	expect_any(__wrap_get_ep_1, clnt);
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("get_ep is not called with non-string field");
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("get_ep is not called with missing required field");
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("get_ep is not called malformed json");
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_ep_1 return error*/
	TEST_CASE(
		"get-ep subcommand fails if get_ep_1 returns empty string interface");
	get_ep_1_ret_val.interface = "";
	expect_function_call(__wrap_get_ep_1);
	will_return(__wrap_get_ep_1, &get_ep_1_ret_val);
	expect_any(__wrap_get_ep_1, argp);
	expect_any(__wrap_get_ep_1, clnt);
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_ep_1 return NULL*/
	TEST_CASE("get-ep subcommand fails if get_ep_1 returns NULl");
	expect_function_call(__wrap_get_ep_1);
	will_return(__wrap_get_ep_1, NULL);
	expect_any(__wrap_get_ep_1, argp);
	expect_any(__wrap_get_ep_1, clnt);
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";

	/* Test cases */
	char *argv1[] = { "delete-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": "10.0.0.1" }) };

	char *argv2[] = { "delete-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": 3, "ip": "10.0.0.1" }) };

	char *argv3[] = { "delete-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3" }) };

	char *argv4[] = { "delete-ep", "-i", "eth0", "-j",
			  QUOTE({ "tunnel_id": "3", "ip": [10.0.0.2] }) };

	struct rpc_trn_endpoint_key_t exp_ep_key = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};

	int delete_ep_1_ret_val = 0;
	/* Test call delete_ep_1 successfully */
	TEST_CASE("delete_ep_1 succeed with well formed endpoint json input");
	expect_function_call(__wrap_delete_ep_1);
	will_return(__wrap_delete_ep_1, &delete_ep_1_ret_val);
	expect_check(__wrap_delete_ep_1, argp, check_ep_key_equal, &exp_ep_key);
	expect_any(__wrap_delete_ep_1, clnt);
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("delete_ep_1 is not called with non-string field");
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("delete_ep_1 is not called with missing required field");
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("delete_ep_1 is not called malformed json");
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_ep_1 return error*/
	TEST_CASE("delete-ep subcommand fails if delete_ep_1 returns error");
	delete_ep_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_ep_1);
	will_return(__wrap_delete_ep_1, &delete_ep_1_ret_val);
	expect_any(__wrap_delete_ep_1, argp);
	expect_any(__wrap_delete_ep_1, clnt);
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_ep_1 return NULL*/
	TEST_CASE("delete-ep subcommand fails if delete_ep_1 returns NULL");
	expect_function_call(__wrap_delete_ep_1);
	will_return(__wrap_delete_ep_1, NULL);
	expect_any(__wrap_delete_ep_1, argp);
	expect_any(__wrap_delete_ep_1, clnt);
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_dft_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";

	/* Test cases */
	char *argv1[] = { "delete-dft", "-i", "eth0", "-j",
			  QUOTE({ "id": "3" }) };

	char *argv2[] = { "delete-dft", "-i", "eth0", "-j",
			  QUOTE({ "id": 3 }) };

	char *argv3[] = { "delete-dft", "-i", "eth0", "-j", QUOTE({ "id 3" }) };

	struct rpc_trn_zeta_key_t exp_dft_key = {
		.interface = itf,
		.id = 3,
	};

	int delete_dft_1_ret_val = 0;
	/* Test call delete_dft_1 successfully */
	TEST_CASE("delete_dft_1 succeed with well formed dft json input");
	expect_function_call(__wrap_delete_dft_1);
	will_return(__wrap_delete_dft_1, &delete_dft_1_ret_val);
	expect_check(__wrap_delete_dft_1, argp, check_zeta_key_equal,
		     &exp_dft_key);
	expect_any(__wrap_delete_dft_1, clnt);
	rc = trn_cli_delete_dft_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse dft input error*/
	TEST_CASE("delete_dft_1 is not called with non-string field");
	rc = trn_cli_delete_dft_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("delete_dft_1 is not called with missing required field");
	rc = trn_cli_delete_dft_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_dft_1 return error*/
	TEST_CASE("delete-dft subcommand fails if delete_dft_1 returns error");
	delete_dft_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_dft_1);
	will_return(__wrap_delete_dft_1, &delete_dft_1_ret_val);
	expect_any(__wrap_delete_dft_1, argp);
	expect_any(__wrap_delete_dft_1, clnt);
	rc = trn_cli_delete_dft_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_dft_1 return NULL*/
	TEST_CASE("delete-dft subcommand fails if delete_dft_1 returns NULL");
	expect_function_call(__wrap_delete_dft_1);
	will_return(__wrap_delete_dft_1, NULL);
	expect_any(__wrap_delete_dft_1, argp);
	expect_any(__wrap_delete_dft_1, clnt);
	rc = trn_cli_delete_dft_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_chain_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";

	/* Test cases */
	char *argv1[] = { "delete-chain", "-i", "eth0", "-j",
			  QUOTE({ "id": "3" }) };

	char *argv2[] = { "delete-chain", "-i", "eth0", "-j",
			  QUOTE({ "id": 3 }) };

	char *argv3[] = { "delete-chain", "-i", "eth0", "-j",
			  QUOTE({ "id 3" }) };

	struct rpc_trn_zeta_key_t exp_chain_key = {
		.interface = itf,
		.id = 3,
	};

	int delete_chain_1_ret_val = 0;
	/* Test call delete_chain_1 successfully */
	TEST_CASE("delete_chain_1 succeed with well formed chain json input");
	expect_function_call(__wrap_delete_chain_1);
	will_return(__wrap_delete_chain_1, &delete_chain_1_ret_val);
	expect_check(__wrap_delete_chain_1, argp, check_zeta_key_equal,
		     &exp_chain_key);
	expect_any(__wrap_delete_chain_1, clnt);
	rc = trn_cli_delete_chain_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse chain input error*/
	TEST_CASE("delete_chain_1 is not called with non-string field");
	rc = trn_cli_delete_chain_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("delete_chain_1 is not called with missing required field");
	rc = trn_cli_delete_chain_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_chain_1 return error*/
	TEST_CASE(
		"delete-chain subcommand fails if delete_chain_1 returns error");
	delete_chain_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_chain_1);
	will_return(__wrap_delete_chain_1, &delete_chain_1_ret_val);
	expect_any(__wrap_delete_chain_1, argp);
	expect_any(__wrap_delete_chain_1, clnt);
	rc = trn_cli_delete_chain_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_chain_1 return NULL*/
	TEST_CASE(
		"delete-chain subcommand fails if delete_chain_1 returns NULL");
	expect_function_call(__wrap_delete_chain_1);
	will_return(__wrap_delete_chain_1, NULL);
	expect_any(__wrap_delete_chain_1, argp);
	expect_any(__wrap_delete_chain_1, clnt);
	rc = trn_cli_delete_chain_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_ftn_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 5;

	char itf[] = "eth0";

	/* Test cases */
	char *argv1[] = { "delete-ftn", "-i", "eth0", "-j",
			  QUOTE({ "id": "3" }) };

	char *argv2[] = { "delete-ftn", "-i", "eth0", "-j",
			  QUOTE({ "id": 3 }) };

	char *argv3[] = { "delete-ftn", "-i", "eth0", "-j", QUOTE({ "id 3" }) };

	struct rpc_trn_zeta_key_t exp_ftn_key = {
		.interface = itf,
		.id = 3,
	};

	int delete_ftn_1_ret_val = 0;
	/* Test call delete_ftn_1 successfully */
	TEST_CASE("delete_ftn_1 succeed with well formed ftn json input");
	expect_function_call(__wrap_delete_ftn_1);
	will_return(__wrap_delete_ftn_1, &delete_ftn_1_ret_val);
	expect_check(__wrap_delete_ftn_1, argp, check_zeta_key_equal,
		     &exp_ftn_key);
	expect_any(__wrap_delete_ftn_1, clnt);
	rc = trn_cli_delete_ftn_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ftn input error*/
	TEST_CASE("delete_ftn_1 is not called with non-string field");
	rc = trn_cli_delete_ftn_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("delete_ftn_1 is not called with missing required field");
	rc = trn_cli_delete_ftn_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_ftn_1 return error*/
	TEST_CASE("delete-ftn subcommand fails if delete_ftn_1 returns error");
	delete_ftn_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_ftn_1);
	will_return(__wrap_delete_ftn_1, &delete_ftn_1_ret_val);
	expect_any(__wrap_delete_ftn_1, argp);
	expect_any(__wrap_delete_ftn_1, clnt);
	rc = trn_cli_delete_ftn_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_ftn_1 return NULL*/
	TEST_CASE("delete-ftn subcommand fails if delete_ftn_1 returns NULL");
	expect_function_call(__wrap_delete_ftn_1);
	will_return(__wrap_delete_ftn_1, NULL);
	expect_any(__wrap_delete_ftn_1, argp);
	expect_any(__wrap_delete_ftn_1, clnt);
	rc = trn_cli_delete_ftn_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

int main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_trn_cli_update_ep_subcmd),
		cmocka_unit_test(test_trn_cli_load_transit_subcmd),
		cmocka_unit_test(test_trn_cli_unload_transit_subcmd),
		cmocka_unit_test(test_trn_cli_get_ep_subcmd),
		cmocka_unit_test(test_trn_cli_delete_ep_subcmd),
		cmocka_unit_test(test_trn_cli_update_dft_subcmd),
		cmocka_unit_test(test_trn_cli_update_ftn_subcmd),
		cmocka_unit_test(test_trn_cli_get_dft_subcmd),
		cmocka_unit_test(test_trn_cli_get_ftn_subcmd),
		cmocka_unit_test(test_trn_cli_delete_dft_subcmd),
		cmocka_unit_test(test_trn_cli_delete_ftn_subcmd),
		cmocka_unit_test(test_trn_cli_get_chain_subcmd),
		cmocka_unit_test(test_trn_cli_delete_chain_subcmd),
		cmocka_unit_test(test_trn_cli_update_chain_subcmd),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
