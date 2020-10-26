#!/bin/bash

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
echo "build path is $BUILD, cleanning up target folder $BUILD/_build/$FLAVOR"
rm -rf $ROOT/build
mkdir -p $ROOT/build/lib

# pull submodules just in case
git submodule update --init --recursive

# Create and Start the build contrainer
docker build -f $ROOT/deploy/build/Dockerfile -t zeta_build --build-arg USER_ID=$(id -u) --build-arg GROUP_ID=$(id -g) $ROOT
docker rm -f zb || true
docker create -v $ROOT:/mnt/host/code -it \
	--privileged \
	--cap-add=NET_ADMIN \
	--cap-add=SYS_PTRACE \
	--security-opt seccomp=unconfined \
	--name zb \
	--workdir /mnt/host/code \
	zeta_build:latest /bin/bash
docker start zb

# Build zeta
echo "--- building zeta ---"
docker exec zb bash -c "cd /mnt/host/code/build && cmake -H/mnt/host/code -B. -DCMAKE_BUILD_TYPE=$FLAVOR -DARCH=$arch -DBUILD_TESTING=On && make $args && ctest -V"