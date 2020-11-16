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

# Checks for status: Provisioned for given object
function get_status() {
    OBJECT=$1

    kubectl get $OBJECT 2> /tmp/kubetctl.err | awk '
    NR==1 {
        for (i=1; i<=NF; i++) {
            f[$i] = i
        }
    }
    { print $f["STATUS"] }
    ' | grep Running > /dev/null

    return $?
}

# Checks for status Provisioned of array of objects
function check_ready() {
    objects=("pods")
    sum=0
    for i in "${objects[@]}"
    do
        get_status $i
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
end=$((SECONDS + $timeout))
echo -n "Waiting for Zeta to come up."
while [[ $SECONDS -lt $end ]]; do
    check_ready || break
done
if [[ $SECONDS -lt $end ]]; then
    echo "Zeta is now ready!"
else
    echo "ERROR: Zeta setup timed out after $timeout seconds!"
    exit 1
fi
