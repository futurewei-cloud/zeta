#!/bin/bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang  <@liangbin>
#
# Summary: Script to deploy postgres service to target k8s cluster
#
set -o errexit

# Get full path of current script no matter where it's placed and invoked
MY_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DEPLOYMENTS_PATH="$MY_PATH/../etc/deployments"

echo "Creating the volume..."
kubectl apply -f $DEPLOYMENTS_PATH/persistent-volume.yml && sleep 1
kubectl apply -f $DEPLOYMENTS_PATH/persistent-volume-claim.yml && sleep 1

echo "Creating the database credentials..."
kubectl apply -f $DEPLOYMENTS_PATH/secret.yml && sleep 1

echo "Creating the postgres deployment and service..."
kubectl apply -f $DEPLOYMENTS_PATH/postgres-deployment.yml && sleep 1
kubectl apply -f $DEPLOYMENTS_PATH/postgres-service.yml && sleep 1
