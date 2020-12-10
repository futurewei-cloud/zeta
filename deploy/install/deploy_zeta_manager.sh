#!/bin/bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang  <@liangbin>
#
# Summary: Script to deploy zeta-manager to target k8s cluster
#
set -o errexit

# Get full path of current script no matter where it's placed and invoked
MY_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DEPLOYMENTS_PATH="$MY_PATH/../etc/deployments"

. $MY_PATH/common.s

echo "Creating the zeta-manager deployment and service..."

REGISTRY="$REG" \
envsubst '$REGISTRY' < $DEPLOYMENTS_PATH/zeta-manager-deployment.yml > $DEPLOYMENTS_PATH/.zeta-manager-deployment.yml

# Delete existing deployment and deploy
delete_pods zeta-manager deployment &>/dev/null || true
kubectl apply -f $DEPLOYMENTS_PATH/.zeta-manager-deployment.yml
kubectl apply -f $DEPLOYMENTS_PATH/zeta-manager-service.yml
kubectl wait --for=condition=ready pod -l app=zeta-manager --timeout=300s

echo -n "Waiting for postgres service ready for connection..."
POD_ZM="$(kubectl get pod --field-selector status.phase=Running -l app=zeta-manager -o jsonpath='{.items[0].metadata.name}')"
end=$((SECONDS + 300))
ready="Not Ready"
while [[ $SECONDS -lt $end ]]; do
    ready="$(kubectl exec $POD_ZM -- cat /tmp/healthy 2>&1 | head -n1)"
    if [ -z "$ready" ]; then
        ready="ready"
        break
    fi
    echo -n "."
    sleep 2
done
if [ "$ready" != "ready" ]; then
    echo "Time out after 300s"
    exit 1
fi
echo


echo "Hooking up to ingress..."
# Make sure Ingress-nginx controller is running
if [[ "$K8S_TYPE" == "kind" ]]; then
    kubectl wait --namespace ingress-nginx \
    --for=condition=ready pod \
    --selector=app.kubernetes.io/component=controller \
    --timeout=300s
elif [[ "$K8S_TYPE" == "microk8s" ]]; then
    kubectl wait --namespace ingress \
    --for=condition=ready pod \
    -l name=nginx-ingress-microk8s \
    --timeout=300s
fi
kubectl apply -f $DEPLOYMENTS_PATH/zeta-ingress.yml


echo "Apply the migrations and seed the database..."

kubectl exec $POD_ZM -- python manage.py recreate_db
kubectl exec $POD_ZM -- python manage.py seed_db
