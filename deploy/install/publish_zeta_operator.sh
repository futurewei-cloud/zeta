#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: Script to Rebuild zeta-opr and zeta-manager images
# Note: publish only if it's local private registry or public registry
#       For microk8s cluster, it will be published later using inmage archive
#
set -o errexit

if [[ ! -z "$STAGE" && "$STAGE" == "user" ]]; then
    # No build and publish
    exit 0
fi

# Get full path of current script no matter where it's placed and invoked
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

. $ROOT/deploy/install/common.sh
get_registry

echo "Package the zeta-operator image and publish to $REG..."
cp -f ${HOME}/.kube/zeta.config ${ROOT}/build/tests/zeta.config
docker image build \
    -t $REG/zeta_opr:latest \
    -f $ROOT/deploy/etc/docker/operator.Dockerfile $ROOT >/dev/null
if [ "$K8S_TYPE" == "microk8s" ]; then
    docker save $REG/zeta_opr:latest > /tmp/zeta_opr.tar
else
    docker image push $REG/zeta_opr:latest >/dev/null
fi

