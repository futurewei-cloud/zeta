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

static int update_net_1_rc = 0;
static int *update_net_1_rc_ptr = &update_net_1_rc;

int __wrap_setrlimit(int resource, const struct rlimit *rlim)
{
	UNUSED(resource);
	UNUSED(rlim);
	return 0;
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

rpc_trn_endpoint_t *__wrap_get_ep_1(rpc_trn_endpoint_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	rpc_trn_endpoint_t *retval = mock_ptr_type(rpc_trn_endpoint_t *);
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

	if (strcmp(ep->interface, c_ep->interface) != 0) {
		return false;
	}

	if (strcmp(ep->hosted_interface, c_ep->hosted_interface) != 0) {
		return false;
	}

	if (strcmp(ep->veth, c_ep->veth) != 0) {
		return false;
	}

	if (memcmp(ep->mac, c_ep->mac, sizeof(char) * 6) != 0) {
		return false;
	}

	if (ep->ip != c_ep->ip) {
		return false;
	}

	if (ep->eptype != c_ep->eptype) {
		return false;
	}

	if (ep->tunid != c_ep->tunid) {
		return false;
	}

	if (ep->remote_ips.remote_ips_len != c_ep->remote_ips.remote_ips_len) {
		return false;
	}

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

	if (strcmp(ep_key->interface, c_ep_key->interface) != 0) {
		return false;
	}

	if (ep_key->tunid != c_ep_key->tunid) {
		return false;
	}

	if (ep_key->ip != c_ep_key->ip) {
		return false;
	}

	return true;
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

int main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_trn_cli_update_ep_subcmd),
		cmocka_unit_test(test_trn_cli_load_transit_subcmd),
		cmocka_unit_test(test_trn_cli_unload_transit_subcmd),
		cmocka_unit_test(test_trn_cli_get_ep_subcmd),
		cmocka_unit_test(test_trn_cli_delete_ep_subcmd)
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
