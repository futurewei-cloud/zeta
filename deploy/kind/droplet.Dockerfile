# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

FROM fwnetworking/zeta_dev:latest
COPY . /var/zeta/
RUN apt-get update
RUN apt-get install -y systemd
RUN ln -snf /var/zeta/build/xdp /trn_xdp
RUN ln -snf /var/zeta/build/bin /trn_bin
RUN ln -snf /sys/fs/bpf /bpffs
CMD /etc/init.d/rpcbind restart && ./trn_bin/transitd
