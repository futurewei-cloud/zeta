// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file test_dmn.c
 * @author Sherif Abdelwahab (@zasherif)
 *         Phu Tran          (@phudtran)
 *
 * @brief dmn unit tests
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
#include <string.h>
#include <sys/resource.h>

#include "trn_log.h"
#include "dmn/trn_transitd.h"
#include "rpcgen/trn_rpc_protocol.h"

int __wrap_setrlimit(int resource, const struct rlimit *rlim)
{
	UNUSED(resource);
	UNUSED(rlim);
	return 0;
}

int __wrap_bpf_map_update_elem(void *map, void *key, void *value,
			       unsigned long long flags)
{
	UNUSED(map);
	UNUSED(key);
	UNUSED(value);
	UNUSED(flags);
	function_called();
	return 0;
}

int __wrap_bpf_map_lookup_elem(void *map, void *key, void *value)
{
	UNUSED(map);
	UNUSED(key);
	struct endpoint_t *endpoint = mock_ptr_type(struct endpoint_t *);
	struct ftn_t *ftn = mock_ptr_type(struct ftn_t *);
	struct dft_t *dft = mock_ptr_type(struct dft_t *);
	function_called();
	if (endpoint != NULL)
		memcpy(value, endpoint, sizeof(*endpoint));
	else if (dft != NULL)
		memcpy(value, dft, sizeof(*dft));
	else if (ftn != NULL)
		memcpy(value, ftn, sizeof(*ftn));
	else
		return 1;
	return 0;
}

int __wrap_bpf_map_delete_elem(void *map, void *key)
{
	UNUSED(map);
	UNUSED(key);

	bool valid = mock_type(bool);
	function_called();

	if (valid)
		return 0;
	return 1;
}

int __wrap_bpf_prog_load_xattr(const struct bpf_prog_load_attr *attr,
			       struct bpf_object **pobj, int *prog_fd)
{
	UNUSED(attr);
	UNUSED(pobj);

	*prog_fd = 1;
	return 0;
}

int __wrap_bpf_set_link_xdp_fd(int ifindex, int fd, __u32 flags)
{
	UNUSED(ifindex);
	UNUSED(fd);
	UNUSED(flags);
	return 0;
}

int __wrap_bpf_obj_get_info_by_fd(int prog_fd, void *info, __u32 *info_len)
{
	UNUSED(prog_fd);
	UNUSED(info);
	UNUSED(info_len);
	return 0;
}

struct bpf_map *__wrap_bpf_map__next(struct bpf_map *map,
				     struct bpf_object *obj)
{
	UNUSED(map);
	UNUSED(obj);
	return (struct bpf_map *)1;
}

int __wrap_bpf_map__fd(struct bpf_map *map)
{
	UNUSED(map);
	return 1;
}

int __wrap_bpf_map__pin(struct bpf_map *map, const char *path)
{
	UNUSED(map);
	UNUSED(path);
	return 0;
}

int __wrap_bpf_map__unpin(struct bpf_map *map, const char *path)
{
	UNUSED(map);
	UNUSED(path);
	return 0;
}

int __wrap_bpf_get_link_xdp_id(int ifindex, __u32 *prog_id, __u32 flags)
{
	UNUSED(ifindex);
	UNUSED(prog_id);
	UNUSED(flags);
	return 0;
}

unsigned int __wrap_if_nametoindex(const char *ifname)
{
	UNUSED(ifname);
	return 1;
}

const char *__wrap_if_indextoname(unsigned int ifindex, char *buf)
{
	UNUSED(buf);
	UNUSED(ifindex);
	if (ifindex != 1)
		return NULL;
	else
		return "lo";
}

struct bpf_object *__wrap_bpf_object__open(const char *path)
{
	UNUSED(path);
	return (struct bpf_object *)1;
}

int __wrap_bpf_create_map(enum bpf_map_type map_type, int key_size,
			  int value_size, int max_entries)
{
	UNUSED(map_type);
	UNUSED(key_size);
	UNUSED(value_size);
	UNUSED(max_entries);
	return 0;
}

int __wrap_bpf_program__fd(const struct bpf_program *prog)
{
	UNUSED(prog);
	return 1;
}

int __wrap_bpf_object__load(struct bpf_object *obj)
{
	UNUSED(obj);
	return 0;
}

struct bpf_map *
__wrap_bpf_object__find_map_by_name(const struct bpf_object *obj,
				    const char *name)
{
	UNUSED(obj);
	UNUSED(name);
	return (struct bpf_map *)1;
}
int __wrap_bpf_map__set_inner_map_fd(struct bpf_map *map, int fd)
{
	UNUSED(map);
	UNUSED(fd);
	return 0;
}

int __wrap_bpf_program__set_xdp(struct bpf_program *prog)
{
	UNUSED(prog);
	return 0;
}

struct bpf_program *__wrap_bpf_program__next(struct bpf_program *prev,
					     const struct bpf_object *obj)
{
	UNUSED(obj);
	if (prev == NULL) {
		return (struct bpf_program *)1;
	} else
		return NULL;
}

void __wrap_bpf_object__close(struct bpf_object *object)
{
	UNUSED(object);
	return;
}
static inline int cmpfunc(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

static int check_ep_equal(rpc_trn_endpoint_t *ep, rpc_trn_endpoint_t *c_ep)
{
	u_int i;

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

static int check_dft_equal(rpc_trn_dft_t *dft, rpc_trn_dft_t *c_dft)
{
	assert_string_equal(dft->interface, c_dft->interface);
	assert_int_equal(dft->id, c_dft->id);
	assert_int_equal(dft->zeta_type, c_dft->zeta_type);

	qsort(dft->table.table_val, dft->table.table_len, sizeof(uint32_t),
	      cmpfunc);

	qsort(c_dft->table.table_val, c_dft->table.table_len, sizeof(uint32_t),
	      cmpfunc);
	u_int i;
	for (i = 0; i < dft->table.table_len; i++) {
		assert_int_equal(c_dft->table.table_val[i],
				 dft->table.table_val[i]);
	}

	return true;
}

static int check_ftn_equal(rpc_trn_ftn_t *ftn, rpc_trn_ftn_t *c_ftn)
{
	assert_string_equal(ftn->interface, c_ftn->interface);
	assert_int_equal(ftn->id, c_ftn->id);
	assert_int_equal(ftn->zeta_type, c_ftn->zeta_type);
	assert_int_equal(ftn->position, c_ftn->position);
	assert_int_equal(ftn->ip, c_ftn->ip);
	assert_int_equal(ftn->next_ip, c_ftn->next_ip);
	assert_string_equal(ftn->mac, c_ftn->mac);
	assert_string_equal(ftn->next_mac, c_ftn->next_mac);

	return true;
}

static void do_lo_xdp_load(void)
{
	rpc_trn_xdp_intf_t xdp_intf;
	char itf[] = "lo";
	char xdp_path[] = "/path/to/xdp/object/file";
	char pcapfile[] = "/path/to/bpf/pinned/map";
	xdp_intf.interface = itf;
	xdp_intf.xdp_path = xdp_path;
	xdp_intf.pcapfile = pcapfile;

	int *rc;
	expect_function_call(__wrap_bpf_map_update_elem);
	expect_function_call(__wrap_bpf_map_update_elem);
	rc = load_transit_xdp_1_svc(&xdp_intf, NULL);
	assert_int_equal(*rc, 0);
}

static void do_lo_xdp_unload(void)
{
	int *rc;
	rpc_intf_t test_itf;
	char itf_str[] = "lo";
	test_itf.interface = itf_str;
	rc = unload_transit_xdp_1_svc(&test_itf, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_dft_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";
	uint32_t table[] = { 0, 1, 2, 3, 4, 5 };
	uint32_t id = 3;
	uint32_t zeta_type = 0;
	struct rpc_trn_dft_t dft = {
		.interface = itf,
		.id = id,
		.zeta_type = zeta_type,
		.table = { .table_len = 6, .table_val = table },
	};

	int *rc;
	expect_function_calls(__wrap_bpf_map_update_elem, 1);
	rc = update_dft_1_svc(&dft, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_ftn_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";
	uint32_t id = 3;
	uint32_t zeta_type = 1;
	uint8_t position = 0;
	uint32_t ip = 0x100000a;
	uint32_t next_ip = 0x200000a;
	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char next_mac[6] = { 7, 8, 9, 1, 2, 3 };

	struct rpc_trn_ftn_t ftn = {
		.interface = itf,
		.id = id,
		.zeta_type = zeta_type,
		.position = position,
		.ip = ip,
		.next_ip = next_ip,
	};
	memcpy(ftn.mac, mac, sizeof(char) * 6);
	memcpy(ftn.next_mac, next_mac, sizeof(char) * 6);

	int *rc;
	expect_function_calls(__wrap_bpf_map_update_elem, 1);
	rc = update_ftn_1_svc(&ftn, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_ep_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	char vitf[] = "veth0";
	char hosted_itf[] = "veth";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t ep1 = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 1,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 3,
	};

	memcpy(ep1.mac, mac, sizeof(char) * 6);

	int *rc;
	expect_function_calls(__wrap_bpf_map_update_elem, 2);
	rc = update_ep_1_svc(&ep1, NULL);
	assert_int_equal(*rc, 0);
}

static void test_get_dft_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	uint32_t table[] = { 0, 1, 2, 3, 4, 5 };
	uint32_t id = 3;
	uint32_t zeta_type = 0;
	struct rpc_trn_dft_t dft1 = {
		.interface = itf,
		.id = id,
		.zeta_type = zeta_type,
		.table = { .table_len = 6, .table_val = table },
	};

	struct rpc_trn_zeta_key_t dft_key1 = {
		.interface = itf,
		.id = id,
		.zeta_type = zeta_type,
	};

	struct dft_t dft_val;
	int i;
	for (i = 0; i < 6; i++) {
		dft_val.table[i] = table[i];
	}
	dft_val.table_len = 6;

	/* Test get_ftn with valid dft_key */
	struct rpc_trn_dft_t *retval;
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, &dft_val);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_dft_1_svc(&dft_key1, NULL);
	assert_true(check_dft_equal(retval, &dft1));

	/* Test get_dft with bad return code from bpf_lookup */
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_dft_1_svc(&dft_key1, NULL);
	assert_true(strlen(retval->interface) == 0);
}

static void test_get_ftn_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	uint32_t id = 3;
	uint32_t zeta_type = 1;
	uint8_t position = 0;
	uint32_t ip = 0x100000a;
	uint32_t next_ip = 0x200000a;
	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char next_mac[6] = { 7, 8, 9, 1, 2, 3 };

	struct rpc_trn_ftn_t ftn1 = {
		.interface = itf,
		.id = id,
		.zeta_type = zeta_type,
		.position = position,
		.ip = ip,
		.next_ip = next_ip,
	};

	memcpy(ftn1.mac, mac, sizeof(char) * 6);
	memcpy(ftn1.next_mac, next_mac, sizeof(char) * 6);

	struct rpc_trn_zeta_key_t ftn_key1 = {
		.interface = itf,
		.id = id,
		.zeta_type = zeta_type,
	};

	struct ftn_t ftn_val;
	ftn_val.position = 0;
	ftn_val.ip = 0x100000a;
	ftn_val.next_ip = 0x200000a;
	memcpy(ftn_val.mac, mac, sizeof(char) * 6);
	memcpy(ftn_val.next_mac, next_mac, sizeof(char) * 6);

	/* Test get_ftn with valid ftn_key */
	struct rpc_trn_ftn_t *retval;
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, &ftn_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ftn_1_svc(&ftn_key1, NULL);
	assert_true(check_ftn_equal(retval, &ftn1));

	/* Test get_ftn with bad return code from bpf_lookup */
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ftn_1_svc(&ftn_key1, NULL);
	assert_true(strlen(retval->interface) == 0);
}

static void test_get_ep_1_svc(void **state)
{
	UNUSED(state);

	char itf[] = "lo";
	char vitf[] = "";
	char hosted_itf[] = "lo";
	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct rpc_trn_endpoint_t ep1 = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 1,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = hosted_itf,
		.veth = vitf,
		.tunid = 3,
	};

	struct rpc_trn_endpoint_key_t ep_key1 = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};

	struct endpoint_t ep_val;
	ep_val.eptype = 1;
	ep_val.nremote_ips = 1;
	ep_val.remote_ips[0] = remote[0];
	ep_val.hosted_iface = 1;
	memcpy(ep_val.mac, ep1.mac, sizeof(ep1.mac));

	struct rpc_trn_endpoint_t ep2 = {
		.interface = itf,
		.ip = 0x100000a,
		.eptype = 0,
		.remote_ips = { .remote_ips_len = 1, .remote_ips_val = remote },
		.hosted_interface = "",
		.veth = vitf,
		.tunid = 0,
	};

	struct rpc_trn_endpoint_key_t ep_key2 = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 0,
	};

	struct endpoint_t ep_val2;
	ep_val2.eptype = 0;
	ep_val2.nremote_ips = 1;
	ep_val2.remote_ips[0] = remote[0];
	ep_val2.hosted_iface = -1;
	memcpy(ep_val2.mac, ep2.mac, sizeof(ep2.mac));

	/* Test get_ep with valid ep_key */
	struct rpc_trn_endpoint_t *retval;
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ep_1_svc(&ep_key1, NULL);
	assert_true(check_ep_equal(retval, &ep1));

	/* Test get_ep substrate with valid ep_key */
	will_return(__wrap_bpf_map_lookup_elem, &ep_val2);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ep_1_svc(&ep_key2, NULL);
	assert_true(check_ep_equal(retval, &ep2));

	/* Test get_ep with bad return code from bpf_lookup */
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ep_1_svc(&ep_key1, NULL);
	assert_false(check_ep_equal(retval, &ep1));

	/* Test get_ep with invalid interface index*/
	ep_val.hosted_iface = 2;
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ep_1_svc(&ep_key1, NULL);
	assert_false(check_ep_equal(retval, &ep1));

	/* Test get_ep with invalid interface*/
	ep_key1.interface = "";
	retval = get_ep_1_svc(&ep_key1, NULL);
	assert_true(strlen(retval->interface) == 0);
}

static void test_delete_dft_1_svc(void **state)
{
	UNUSED(state);
	int *rc;
	char itf[] = "lo";
	uint32_t id = 3;
	uint32_t zeta_type = 1;
	uint32_t table[] = { 0, 1, 2, 3, 4, 5 };
	struct rpc_trn_zeta_key_t dft_key = {
		.interface = itf,
		.id = id,
		.zeta_type = zeta_type,
	};

	/* Test delete_dft_1 with valid dft_key */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_dft_1_svc(&dft_key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_dft_1 with invalid dft_key */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_dft_1_svc(&dft_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);

	/* Test delete_dft_1 with invalid interface*/
	dft_key.interface = "";
	rc = delete_dft_1_svc(&dft_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_delete_ftn_1_svc(void **state)
{
	UNUSED(state);
	int *rc;
	char itf[] = "lo";
	uint32_t id = 3;
	uint32_t zeta_type = 1;

	struct rpc_trn_zeta_key_t ftn_key = {
		.interface = itf,
		.id = id,
		.zeta_type = zeta_type,
	};

	/* Test delete_ftn_1 with valid ftn_key */
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_ftn_1_svc(&ftn_key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_ftn_1 with invalid ftn_key */
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_ftn_1_svc(&ftn_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);

	/* Test delete_ftn_1 with invalid interface*/
	ftn_key.interface = "";
	rc = delete_ftn_1_svc(&ftn_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

static void test_delete_ep_1_svc(void **state)
{
	UNUSED(state);
	char itf[] = "lo";
	struct rpc_trn_endpoint_key_t ep_key = {
		.interface = itf,
		.ip = 0x100000a,
		.tunid = 3,
	};
	int *rc;

	uint32_t remote[] = { 0x200000a };
	char mac[6] = { 1, 2, 3, 4, 5, 6 };

	struct endpoint_t ep_val;
	ep_val.eptype = 1;
	ep_val.nremote_ips = 1;
	ep_val.remote_ips[0] = remote[0];
	ep_val.hosted_iface = 1;
	memcpy(ep_val.mac, mac, sizeof(mac));

	/* Test delete_ep_1 with valid ep_key */
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_ep_1_svc(&ep_key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_ep_1 with invalid ep_key */
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_ep_1_svc(&ep_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);

	/* Test delete_ep_1 with invalid interface*/
	ep_key.interface = "";
	rc = delete_ep_1_svc(&ep_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

/**
 * This is run once before all group tests
 */
static int groupSetup(void **state)
{
	UNUSED(state);
	TRN_LOG_INIT("transitd_unit");
	trn_itf_table_init();
	do_lo_xdp_load();
	return 0;
}

/**
 * This is run once after all group tests
 */
static int groupTeardown(void **state)
{
	UNUSED(state);
	do_lo_xdp_unload();
	trn_itf_table_free();
	TRN_LOG_CLOSE();
	return 0;
}

int main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_update_ep_1_svc),
		cmocka_unit_test(test_get_ep_1_svc),
		cmocka_unit_test(test_delete_ep_1_svc),
		cmocka_unit_test(test_update_dft_1_svc),
		cmocka_unit_test(test_update_ftn_1_svc),
		cmocka_unit_test(test_get_dft_1_svc),
		cmocka_unit_test(test_get_ftn_1_svc),
		cmocka_unit_test(test_delete_dft_1_svc),
		cmocka_unit_test(test_delete_ftn_1_svc),
	};

	int result = cmocka_run_group_tests(tests, groupSetup, groupTeardown);

	return result;
}
