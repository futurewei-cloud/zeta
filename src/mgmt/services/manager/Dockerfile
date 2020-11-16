# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: Zeta-manager service Dockerfile

# base image
FROM python:3.8.1-slim

# install netcat
RUN apt-get update && \
    apt-get -y install netcat && \
    apt-get clean

# set working directory
WORKDIR /var/zeta

# Add app
COPY . /var/zeta

# Setup app
RUN pip install -r requirements.txt && \
    chmod +x entrypoint.sh

# Run app
CMD ["/var/zeta/entrypoint.sh"]
