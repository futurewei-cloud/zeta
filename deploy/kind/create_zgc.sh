#!/bin/bash

# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Bin Liang         <@liangbin>
#

kind_network='kind'
zgc_network="zgc_network"
tenant_network="tenant_network"

# Get full path of current ROOT no matter where it's placed and invoked
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

. $ROOT/deploy/install/common.sh

STAGE=${1:-dev}
DROPLET_NODES=${2:-12}
reg_name='local-kind-registry'
reg_port='5000'

# Remove existing zeta-nodes containers
DROPLETS=$(docker ps -a --format "{{.Names}}" | grep zeta-node)
echo "Deleting existing zeta-node containers"
docker stop $DROPLETS > /dev/null 2>&1
docker rm -f $DROPLETS > /dev/null 2>&1
docker container prune -f > /dev/null 2>&1

if [[ ! -z "$STAGE" && "$STAGE" != "user" ]]; then
  echo "Rebuild and publish zeta_droplet image to $REG..."
  docker image build -t $REG/zeta_droplet:latest -f ${ROOT}/deploy/kind/droplet.Dockerfile $ROOT >/dev/null
  docker image push $REG/zeta_droplet:latest >/dev/null
fi

# Bring up droplets
echo "Creating $DROPLET_NODES containers as ZGC nodes"
for ((i=1; i<=$DROPLET_NODES; i++));
do
    echo -e "Creating zeta-node-$i"
    docker run -d \
        --privileged \
        --cap-add=NET_ADMIN \
        --cap-add=SYS_PTRACE \
        --security-opt seccomp=unconfined \
        --pid=host \
        --network=$kind_network \
        --name zeta-node-$i \
        $REG/zeta_droplet:latest
done

# Populate default zgc for KIND deployment
manager_ip=$(kubectl get node $(kubectl get pods -o wide | grep zeta-manager | awk '{print $7}' | cut -d/ -f1) -o wide | grep kind | awk '{print $6}' | cut -d/ -f1)

response=$(curl -H 'Content-Type: application/json' -X POST \
    -d '{"name":"zgc0",
        "description":"zgc0",
        "ip_start":"20.0.0.1",
        "ip_end":"20.0.0.16",
        "port_ibo":"8300"}' \
    $manager_ip:80/zgcs)

zgc_id=$(echo $response | sed 's/\\[tn]//g' | cut -d':' -f 7 | tr -d '"}' | xargs)
echo "REST API: Test ZGC zgc0 created with UUID: $zgc_id returned"

# Register ZGC nodes containers in KIND deployment
inf_control="eth0"
inf_zgc="eth1"
inf_tenant="eth2"
nodes=($(docker ps | grep -o 'zeta-node-[0-9]\+'))
for node in "${nodes[@]}"
do
    docker network connect $zgc_network $node
    docker network connect $tenant_network $node
    ip_control=$(docker exec $node ip addr show $inf_control | grep "inet\\b" | awk '{print $2}' | cut -d/ -f1)
    ip_tenant=$(docker exec $node ip addr show $inf_tenant | grep "inet\\b" | awk '{print $2}' | cut -d/ -f1)
    mac_tenant=$(docker exec $node ip addr show $inf_tenant | grep "link/ether\\b" | awk '{print $2}' | cut -d/ -f1)
    ip_zgc=$(docker exec $node ip addr show $inf_zgc | grep "inet\\b" | awk '{print $2}' | cut -d/ -f1)
    mac_zgc=$(docker exec $node ip addr show $inf_zgc | grep "link/ether\\b" | awk '{print $2}' | cut -d/ -f1)
    body='{"zgc_id":"'"$zgc_id"'",
    "description":"'"$node"'",
    "name":"'"$node"'",
    "ip_control":"'"$ip_control"'",
    "id_control":"ubuntu",
    "pwd_control":"changeme",
    "inf_tenant":"'"$inf_tenant"'",
    "mac_tenant":"'"$mac_tenant"'",
    "inf_zgc":"'"$inf_zgc"'",
    "mac_zgc":"'"$mac_zgc"'"}'
    curl -H 'Content-Type: application/json' -X POST -d "$body" $manager_ip:80/nodes
    echo "REST API: ZGC node $node registered to zgc0"
done

objects=("dfts" "chains" "ftns" "fwds" "droplets")
timeout=120
end=$((SECONDS + $timeout))
echo -n "Waiting for operator objects to be provisioned"
while [[ $SECONDS -lt $end ]]; do
    check_ready "${objects[@]}" "Provisioned" || break
done
echo
if [[ $SECONDS -lt $end ]]; then
    echo "Operator objects are ready!"
else
    echo "ERROR: Operator object status check timed out after $timeout seconds!"
    exit 1
fi
