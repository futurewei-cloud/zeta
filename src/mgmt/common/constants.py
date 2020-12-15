# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:The above copyright
# notice and this permission notice shall be included in all copies or
# substantial portions of the Software.THE SOFTWARE IS PROVIDED "AS IS",
# WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Constants
group = 'zeta.com'
version = 'v1'


class CONSTANTS:
    TRAN_SUBSTRT_EP = 0
    TRAN_SIMPLE_EP = 1
    TRAN_SCALED_EP = 2
    TRAN_GATEWAY_EP = 3
    ON_XDP_TX = "ON_XDP_TX"
    ON_XDP_PASS = "ON_XDP_PASS"
    ON_XDP_REDIRECT = "ON_XDP_REDIRECT"
    ON_XDP_DROP = "ON_XDP_DROP"
    ON_XDP_SCALED_EP = "ON_XDP_SCALED_EP"
    IPPROTO_TCP = "6"
    IPROTO_UDP = "17"


class OBJ_STATUS:
    obj_status_init = 'Init'
    obj_status_provisioned = 'Provisioned'


class OBJ_DEFAULTS:
    kopf_max_retries = 10

    default_dft = "dft0"
    default_n_chains = 3
    default_n_replicas = 3

    # Some good prime numbers: 101, 211, 307, 401, 503, 601, 701, 809, 907, 1009, 2003
    # at least 100x greater than size and prime.
    default_maglev_table_size = 307

    tenant_net = "tenant"
    zgc_net = "zgc"


class RESOURCES:
    dfts = "dfts"
    chains = "chains"
    ftns = "ftns"
    fwds = "fwds"
    droplets = "droplets"


class KIND:
    dft = "Dft"
    chain = "Chain"
    ftn = "Ftn"
    fwd = "Fwd"
    droplet = "Droplet"
    ep = "Endpoint"
    node = "Node"


class LAMBDAS:
    status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.obj_status_init
    status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.obj_status_provisioned
