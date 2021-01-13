#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang  <@liangbin>
#
# Summary: Zeta-manager service Dockerfile
#
# base image
FROM fwnetworking/python_base:latest

ENV PYTHONUNBUFFERED 1

# set working directory
WORKDIR /opt/zeta/manager

# Add app
COPY build/manager /opt/zeta/manager/
COPY build/bin /opt/zeta/bin/

# install netcat and manager
RUN ln -snf /opt/zeta/bin /trn_bin && \
    apt-get install -y rsyslog && \
    pip3 install /opt/zeta/manager/

# Run app in shell format
CMD /etc/init.d/rsyslog restart && /opt/zeta/manager/entrypoint.sh
