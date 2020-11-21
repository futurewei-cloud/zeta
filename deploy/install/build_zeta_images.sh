#!/bin/bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang  <@liangbin>
#
# Summary: Rebuild and tag Zeta service images for deployment
#
set -o errexit

# Bypass if it's "user" stage
if [[ ! -z "$STAGE" && "$STAGE" == "user" ]]; then
    exit 0
fi

# Get full path of current script no matter where it's placed and invoked
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

echo "Rebuild zeta-operator image..."
docker image build -t $REG/zeta_opr:latest \
    -f $ROOT/deploy/etc/docker/operator.Dockerfile $ROOT >/dev/null

echo "Rebuild zeta-manager image..."
docker image build -t $REG/zeta_manager:latest \
    -f $ROOT/deploy/etc/docker/manager.Dockerfile \
    $ROOT/src/mgmt/services/manager >/dev/null

if [[ "$REG" == "localhost:32000" ]]; then
    echo "Archiving Zeta service images..."
    docker save $REG/zeta_opr:latest $REG/zeta_manager:latest > /tmp/zeta_images.tar
fi
