#!/bin/bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang  <@liangbin>
#
# Summary: Script to check deployment of Zeta services on target k8s cluster
#
set -o errexit

# Checks for status for given object
function get_status() {
    OBJECT=$1
    STATUS=$2
    kubectl get $OBJECT 2> /tmp/kubetctl.err | awk '
    NR==1 {
        for (i=1; i<=NF; i++) {
            f[$i] = i
        }
    }
    { print $f["STATUS"] }
    ' | grep $STATUS > /dev/null

    return $?
}

# Checks for status Provisioned of array of objects
function check_ready() {
    sum=0
    for i in $1
    do
        get_status $i ${@: -1}
        let sum+=$((sum + $?))
    done
    if [[ $sum == 0 ]]; then
        return 1
    else
        sleep 2
        echo -n "."
        return 0
    fi
}

timeout=120
objects=("pods")
end=$((SECONDS + $timeout))
echo -n "Waiting for Operator to come up."
while [[ $SECONDS -lt $end ]]; do
    check_ready "${objects[@]}" "Running" || break
done
echo
if [[ $SECONDS -lt $end ]]; then
    echo "Zeta services are ready!"
else
    echo "ERROR: Zeta services deployment timed out after $timeout seconds!"
    exit 1
fi

manager_ip=$(kubectl get node $(kubectl get pods -o wide | grep zeta-manager | awk '{print $7}' | cut -d/ -f1) -o wide | grep kind | awk '{print $6}' | cut -d/ -f1)

response=$(curl -H 'Content-Type: application/json' -X POST \
    -d '{"name":"zgc0",
          "description":"zgc0",
          "ip_start":"20.0.0.0",
          "ip_end":"20.0.0.255",
          "port_ibo":"8300"}' \
      $manager_ip:80/zgcs)

zgc_id=$(echo $response | sed 's/\\[tn]//g' | cut -d':' -f 7 | tr -d '"}' | xargs)
tenant_network="tenant_network"
zgc_network="zgc_network"

if [[ "$K8S_TYPE" == "kind" ]]; then

    # Register node containers in KIND deployment
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
    done

    objects=("dfts" "chains" "ftns" "fwds" "droplets")
    end=$((SECONDS + $timeout))
    echo -n "Waiting for default objects to be provisioned"
    while [[ $SECONDS -lt $end ]]; do
        check_ready "${objects[@]}" "Provisioned" || break
    done
    echo
    if [[ $SECONDS -lt $end ]]; then
        echo "Objects now ready!"
    else
        echo "ERROR: Object deployment timed out after $timeout seconds!"
        exit 1
    fi
fi
