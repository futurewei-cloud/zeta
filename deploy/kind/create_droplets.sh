#!/bin/bash

# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

DOCKER_ACC=${1:-fwnetworking}
NODES=${2:-1}

DROPLETS=$(docker ps -a --format "{{.Names}}" | grep zeta-droplet)
NETWORK="droplet_network"

# Remove existing droplet containers and tenant network
docker network rm $NETWORK > /dev/null 2>&1
echo "Deleting existing zeta-droplets containers"
docker stop $DROPLETS > /dev/null 2>&1
docker rm -f $DROPLETS > /dev/null 2>&1

docker image build -t $DOCKER_ACC/zeta-droplet:latest -f ${ROOT}/deploy/kind/droplet.Dockerfile $ROOT

docker network create -d bridge \
  --subnet=20.0.0.0/8 \
  --gateway=20.0.0.1 \
  --opt com.docker.network.driver.mtu=9000 \
  $NETWORK

for ((i=1; i<=NODES; i++));
do
    echo -e "Creating zeta-droplet-$i"
    docker run -d \
        --privileged \
        --cap-add=NET_ADMIN \
        --cap-add=SYS_PTRACE \
        --security-opt seccomp=unconfined \
        --pid=host \
        --network=$NETWORK \
        --name zeta-droplet-$i \
        $DOCKER_ACC/zeta-droplet:latest
done
