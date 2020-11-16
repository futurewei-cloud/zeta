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

# Checks for status for given object
function get_status() {
    OBJECT=$1
    STATUS=$2
    kubectl get $OBJECT 2> /tmp/kubetctl.err | awk '
    NR==1 {
        for (i=1; i<=NF; i++) {
            f[$i] = i
        }
    }
    { print $f["STATUS"] }
    ' | grep $STATUS > /dev/null

    return $?
}

# Checks for status Provisioned of array of objects
function check_ready() {
    sum=0
    for i in $1
    do
        get_status $i ${@: -1}
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

# make

# Get full path of current ROOT no matter where it's placed and invoked
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." >/dev/null 2>&1 && pwd )"
KINDCONF="${ROOT}/build/tests/kind/config"
ZETACONF="${ROOT}/build/tests/zeta.config"
KINDHOME="${HOME}/.kube/config"
USER=${1:-user}
KUBE_NODES=${2:-1}
DROPLET_NODES=${3:-12}
timeout=120

kind delete cluster
docker stop local-kind-registry 2> /dev/null
docker rm local-kind-registry
docker network rm kind 2> /dev/null
# All interfaces in the network have an MTU of 9000 to
# simulate a real datacenter. Since all container traffic
# goes through the docker bridge, we must ensure the bridge
# interfaces also has the same MTU to prevent ip fragmentation.
docker network create -d bridge \
  --subnet=172.18.0.0/16 \
  --gateway=172.18.0.1 \
  --opt com.docker.network.driver.mtu=9000 \
  kind

if [[ "$USER" == "dev" ]]; then
    DOCKER_ACC="localhost:5000"
else
    DOCKER_ACC="fwnetworking"
fi

source ${ROOT}/deploy/kind/create_droplets.sh $DOCKER_ACC $DROPLET_NODES

docker image build -t $DOCKER_ACC/kindnode:latest -f ${ROOT}/deploy/kind/Dockerfile $ROOT
source ${ROOT}/deploy/kind/create_cluster.sh $KINDCONF $USER $KUBE_NODES

api_ip=`docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' kind-control-plane`
sed "s/server: https:\/\/127.0.0.1:[[:digit:]]\+/server: https:\/\/$api_ip:6443/" $KINDCONF > $ZETACONF
ln -snf $KINDCONF $KINDHOME
source $ROOT/deploy/install/deploy_mgmt.sh $USER $DOCKER_ACC

objects=("pods")
end=$((SECONDS + $timeout))
echo -n "Waiting for Operator to come up."
while [[ $SECONDS -lt $end ]]; do
    check_ready "${objects[@]}" "Running" || break
done
echo
if [[ $SECONDS -lt $end ]]; then
    echo "Operator now ready!"
else
    echo "ERROR: Operator deployment timed out after $timeout seconds!"
    exit 1
fi

source ${ROOT}/deploy/kind/register_droplets.sh $DROPLET_NODES

objects=("dfts" "chains" "ftns" "fwds" "droplets")

end=$((SECONDS + $timeout))
echo -n "Waiting for default objects to be provisioned"
while [[ $SECONDS -lt $end ]]; do
    check_ready "${objects[@]}" "Provisioned" || break
done
echo
if [[ $SECONDS -lt $end ]]; then
    echo "Objects now ready!"
else
    echo "ERROR: Object deployment timed out after $timeout seconds!"
    exit 1
fi
