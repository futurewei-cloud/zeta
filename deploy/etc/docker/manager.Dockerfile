#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang  <@liangbin>
#
# Summary: Zeta-manager service Dockerfile
#
# base image
FROM python:3.8.1-slim

# install netcat
RUN apt-get update && \
    apt-get -y install netcat && \
    apt-get clean

# set working directory
WORKDIR /opt/zeta/manager

# Add app
COPY build/manager /opt/zeta/manager
COPY build/bin /opt/zeta/bin

# Setup app
RUN pip3 install /opt/zeta/manager/

#RUN pip install -r requirements.txt && \
#    chmod +x /usr/src/app/entrypoint.sh

# Run app
CMD ["/opt/zeta/manager/entrypoint.sh"]
