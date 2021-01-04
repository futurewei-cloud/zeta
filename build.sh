#!/bin/bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang         <@liangbin>
#
# Summary: Top level script to build and unit test zeta project in container
#

# Check if we are building flavor: debug vs release
if [ "$1" == "release" ]; then
  FLAVOR="release"
else
  FLAVOR="debug"
fi

args="${@:2}"
arch="$(uname -m)"

# Get full path of build.sh script no matter where it's placed and invoked
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
echo "Root path is $ROOT, cleanning up target folder $ROOT/build for $FLAVOR build"
rm -rf $ROOT/build
mkdir -p $ROOT/build

# pull submodules just in case
git submodule update --init --recursive

# Create and Start the build contrainer
docker build -f $ROOT/deploy/build/Dockerfile -t zeta_build:latest \
	--build-arg USER_NAME=$(id -u -n) \
	--build-arg USER_ID=$(id -u) \
	--build-arg GROUP_ID=$(id -g) \
	$ROOT
docker rm -f zb || true
docker create \
	-v /usr/include/linux:/usr/include/linux \
	-v $ROOT:/mnt/host/code \
	-it \
	--privileged \
	--cap-add=NET_ADMIN \
	--cap-add=SYS_PTRACE \
	--security-opt seccomp=unconfined \
	--name zb \
	--workdir /mnt/host/code \
	zeta_build:latest /bin/bash
docker start zb

# Build and Unit Test zeta
echo "--- building zeta ---"
CMKAE_OPT="-H/mnt/host/code -B."
CMKAE_OPT+=" -DCMAKE_BUILD_TYPE=$FLAVOR"
CMKAE_OPT+=" -DARCH=$arch"
CMKAE_OPT+=" -DBUILD_TESTING=On"

BUILD_TASK="cd /mnt/host/code/build"
BUILD_TASK+=" && cmake ${CMKAE_OPT}"
BUILD_TASK+=" && make $args"
BUILD_TASK+=" && ctest -V"
docker exec zb bash -c "${BUILD_TASK}"
