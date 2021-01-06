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

# Get full path of current script no matter where it's placed and invoked
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

. $ROOT/deploy/install/common.sh

timeout=120
objects=("pods")
end=$((SECONDS + $timeout))
echo -n "Waiting for Zeta control plane services to come up."
while [[ $SECONDS -lt $end ]]; do
    check_ready "${objects[@]}" "Running" || break
done
echo
if [[ $SECONDS -lt $end ]]; then
    echo "Zeta control plane services are ready!"
else
    echo "ERROR: Zeta control plane services deployment timed out after $timeout seconds!"
    exit 1
fi

sleep 10s
