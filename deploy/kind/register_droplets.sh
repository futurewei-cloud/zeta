#!/bin/bash

# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

NODES=${1:-1}

FINAL_NODES=""
ITF="eth1"
for ((i=1; i<=NODES; i++));
do
    docker network connect kind zeta-droplet-$i
    ip=$(docker exec zeta-droplet-$i ip addr show $ITF | grep "inet\\b" | awk '{print $2}' | cut -d/ -f1)
    mac=$(docker exec zeta-droplet-$i ip addr show $ITF | grep "link/ether\\b" | awk '{print $2}' | cut -d/ -f1)
    cat <<EOF | kubectl create  -f=-
    apiVersion: zeta.com/v1
    kind: Droplet
    metadata:
        name: zeta-droplet-$i
    spec:
        mac: $mac
        ip: $ip
        itf: $ITF
        status: Init
EOF
done
