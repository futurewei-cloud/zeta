#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Phu Tran  <@phudtran>
#          Bin Liang <@liangbin>
#
# Summary: Script to Rebuild and publish zetanode image to correct registry
#
set -o errexit

if [[ ! -z "$STAGE" && "$STAGE" == "user" ]]; then
    # No build or publish
    exit 0
fi

# Get full path of current script no matter where it's placed and invoked
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

echo "Package the zetanode image and publish to $REG..."
docker image build -t $REG/zetanode:latest -f $ROOT/deploy/kind/Dockerfile $ROOT >/dev/null
docker image push $REG/zetanode:latest >/dev/null
