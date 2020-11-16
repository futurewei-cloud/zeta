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

if [[ "$K8S_TYPE" == "kind" ]]; then

    # Register droplet containers in KIND deployment
    ITF="eth1"
    droplets=($(docker ps | grep -o 'zeta-droplet-[0-9]\+'))

    for droplet in "${droplets[@]}"
    do
        docker network connect kind $droplet
        ip=$(docker exec $droplet ip addr show $ITF | grep "inet\\b" | awk '{print $2}' | cut -d/ -f1)
        mac=$(docker exec $droplet ip addr show $ITF | grep "link/ether\\b" | awk '{print $2}' | cut -d/ -f1)
        cat <<EOF | kubectl create  -f=-
        apiVersion: zeta.com/v1
        kind: Droplet
        metadata:
            name: $droplet
        spec:
            mac: $mac
            ip: $ip
            itf: $ITF
            status: Init
EOF
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
