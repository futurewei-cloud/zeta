# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Bin Liang <@liangbin>
#
# Summary: Base image for Zeta droplet node container in KIND deployment as 
#          fwnetworking/zeta_droplet:latest
# 

FROM ubuntu:18.04@sha256:646942475da61b4ce9cc5b3fadb42642ea90e5d0de46111458e100ff2c7031e6

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    systemd \
    rpcbind \
    rsyslog \
    libboost-all-dev \
    iproute2  \
    net-tools \
    iputils-ping \
    ethtool \
    curl \
    && apt-get clean \
    && ln -snf /sys/fs/bpf /bpffs
