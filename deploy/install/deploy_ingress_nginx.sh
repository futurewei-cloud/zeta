#!/bin/bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang  <@liangbin>
#
# Summary: Script to deploy ingress-nginx controller to target k8s cluster
#
set -o errexit

echo "Create Nginx Ingress Controller..."
kubectl apply -f https://raw.githubusercontent.com/kubernetes/ingress-nginx/master/deploy/static/provider/kind/deploy.yaml >/dev/null

# Remove unnecessary validation through webhooks when new ingresses are added
kubectl delete -A ValidatingWebhookConfiguration ingress-nginx-admission &>/dev/null
