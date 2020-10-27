# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang         <@liangbin>
#
# Summary: Customize zeta_dev container for local build environments

FROM fwnetworking/zeta_dev:latest

# Set container user to match host $USER to avoid messing up shared folders
ARG USER_ID
ARG GROUP_ID

RUN addgroup --gid $GROUP_ID user && \
    adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID user

USER user
