# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>

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
#
# Summary: Base image for Kind node container in KIND deployment as 
#          fwnetworking/kindnode:zeta
# 

FROM kindest/node:v1.18.2
RUN apt-get update && \
    apt-get install -y \
        apt-utils \
        sudo \
        vim \
        rpcbind \
        rsyslog \
        libelf-dev \
        iproute2 \
        net-tools \
        iputils-ping \
        ethtool \
        curl \
        python3 \
        python3-pip \
        tcpdump && \
    pip3 install \
        PyYAML \
        kopf \
        netaddr \
        ipaddress \
        pyroute2 \
        rpyc \
        kubernetes==11.0.0 \
        luigi==2.8.12 \
        grpcio \
        protobuf \
        fs && \
    mkdir -p /opt/cni/bin /etc/cni/net.d && \
    ln -snf /sys/fs/bpf /bpffs
