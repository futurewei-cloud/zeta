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

#FROM python:3.7
FROM python:3.8.1-slim
RUN apt-get update -y && apt-get install -y \
    net-tools \
    ethtool \
    sudo
RUN pip3 install \
    PyYAML \
    kopf \
    netaddr \
    ipaddress \
    pyroute2 \
    rpyc \
    kubernetes==11.0.0 \
    luigi==2.8.12 \
    grpcio \
    protobuf \
    fs
RUN mkdir -p /var/{run/luigi,log,lib} /etc/luigi
