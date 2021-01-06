#!/bin/bash

# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>
#          Bin Liang         <@liangbin>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:The above copyright
# notice and this permission notice shall be included in all copies or
# substantial portions of the Software.THE SOFTWARE IS PROVIDED "AS IS",
# WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.


# Enforce minimal KIND version to support using "kind" network with customized MTU instead of the "bridge" 
kindver="$(kind version | head -n1 | cut -d" " -f2)"
kindrequired="v0.8.0"
if [ "$(printf '%s\n' "$kindrequired" "$kindver" | sort -V | head -n1)" != "$kindrequired" ]; then 
    echo " --> Minimal KIND version is $kindrequired, running version is $kindver, Please upgrade before proceed!"
    exit 1
fi
kind_network='kind'
zgc_network="zgc_network"
tenant_network="tenant_network"

# Get full path of current ROOT no matter where it's placed and invoked
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"
KINDCONF="${HOME}/.kube/config.kind"
ZETACONF="${HOME}/.kube/zeta.config"
KINDHOME="${HOME}/.kube/config"

STAGE=${1:-dev}
KUBE_NODES=${2:-1}
reg_name='local-kind-registry'
reg_port='5000'

# Clean up cluster
kind delete cluster
docker network disconnect $kind_network $reg_name &>/dev/null
docker network rm $kind_network &>/dev/null
# Remove existing droplet containers and tenant network
DROPLETS=$(docker ps -a --format "{{.Names}}" | grep zeta-node)
docker network rm $zgc_network > /dev/null 2>&1
docker network rm $tenant_network > /dev/null 2>&1
echo "Deleting existing zeta-node containers"
docker stop $DROPLETS > /dev/null 2>&1
docker rm -f $DROPLETS > /dev/null 2>&1
docker container prune -f > /dev/null 2>&1
docker network prune -f > /dev/null 2>&1
docker volume prune -f > /dev/null 2>&1

# All interfaces in the network have an MTU of 9000 to
# simulate a real datacenter. Since all container traffic
# goes through the docker bridge, we must ensure the bridge
# interfaces also has the same MTU to prevent ip fragmentation.
docker network create -d bridge \
  --subnet=172.18.0.0/16 \
  --gateway=172.18.0.1 \
  --opt com.docker.network.driver.mtu=9000 \
  $kind_network >/dev/null

docker network create -d bridge \
  --subnet=10.0.0.0/24 \
  --gateway=10.0.0.1 \
  --opt com.docker.network.driver.mtu=9000 \
  $zgc_network >/dev/null

docker network create -d bridge \
  --subnet=20.0.0.0/8 \
  --gateway=20.0.0.1 \
  --opt com.docker.network.driver.mtu=9000 \
  $tenant_network >/dev/null

# Attach local registry to kind_network
if [[ "$STAGE" == "development" ]]; then

  # create local registry container unless it already exists
  running="$(docker inspect -f '{{.State.Running}}' ${reg_name})"
  if [[ "${running}" != 'true' ]]; then
      echo "Install local registry..."
      docker run \
          -d --restart=always -p "${reg_port}:5000" --name "${reg_name}" \
          --network=$kind_network \
          registry:2 >/dev/null
  else
      # Make sure the existing local registry is connected with KIND network 
      docker network connect $kind_network $reg_name >/dev/null
  fi

  reg_ip="$(docker inspect -f '{{.NetworkSettings.Networks.kind.IPAddress}}' ${reg_name})"
  PATCH="containerdConfigPatches:
  - |-
    [plugins.\"io.containerd.grpc.v1.cri\".registry.mirrors.\"localhost:${reg_port}\"]
    endpoint = [\"http://${reg_name}:${reg_port}\"]"
else
  # Use container images in dockeker hub public registry
  PATCH=""
fi

if [[ ! -z "$STAGE" && "$STAGE" != "user" ]]; then
  echo "Rebuild and publish zeta_node image to $REG..."
  docker image build -t $REG/zeta_node:latest -f $ROOT/deploy/kind/Dockerfile $ROOT >/dev/null
  docker image push $REG/zeta_node:latest >/dev/null
fi

NODE_TEMPLATE="  - role: worker
    image: ${REG}/zeta_node:latest
"
FINAL_NODES=""

for ((i=1; i<=NODES; i++));
do
  FINAL_NODES=$FINAL_NODES$NODE_TEMPLATE
done

# create a cluster with the local registry enabled in containerd and n Nodes.
cat <<EOF | kind create cluster --name kind --kubeconfig ${KINDCONF} --config=-
kind: Cluster
apiVersion: kind.x-k8s.io/v1alpha4
${PATCH}
nodes:
  - role: control-plane
    image: ${REG}/zeta_node:latest
    kubeadmConfigPatches:
    - |
      kind: InitConfiguration
      nodeRegistration:
        kubeletExtraArgs:
          node-labels: "ingress-ready=true"
    extraPortMappings:
    - containerPort: 80
      hostPort: 8080
      protocol: TCP
    - containerPort: 443
      hostPort: 443
      protocol: TCP

${FINAL_NODES}
EOF

api_ip=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' kind-control-plane`
sed "s/server: https:\/\/127.0.0.1:[[:digit:]]\+/server: https:\/\/$api_ip:6443/" $KINDCONF > $ZETACONF
ln -snf $KINDCONF $KINDHOME

if [[ $STAGE == "development" ]]; then
  for node in $(kind get nodes); do
    kubectl annotate node "${node}" "kind.x-k8s.io/registry=${REG}" >/dev/null
  done
  # Document the local registry
  # https://github.com/kubernetes/enhancements/tree/master/keps/sig-cluster-lifecycle/generic/1755-communicating-a-local-registry
  cat <<EOF | kubectl apply -f -
  apiVersion: v1
  kind: ConfigMap
  metadata:
    name: local-registry-hosting
    namespace: kube-public
  data:
    localRegistryHosting.v1: |
      host: "localhost:${reg_port}"
EOF
fi
