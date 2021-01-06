#!/usr/bin/env bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang  <@liangbin>
#
# Summary: script to deploy network and KVM vms
#

#set -x

# ask if script runs without sudo or root priveleges
if [ "$(whoami)" != "root" ]; then
    echo " you need to run this script as root"
    exit 1
fi

if [ "$#" -eq 0 ]; then
    echo " Please add parameter to tell me how many VMs to deploy"
    exit 1
fi

CNT=$1

echo "========= Step 1: check dependencies ..."
MISSING=""
FOUND=""
checkdep() {
    local exe="$1" package="$2" upstream="$3"
    if command -v "$1" >/dev/null 2>&1; then
        FOUND="${FOUND:+${FOUND} }$exe"
        return "0"
    fi
    MISSING=${MISSING:+${MISSING }$package}
    echo "missing $exe."
    echo "  It can be installed in package: $package"
    [ -n "$upstream" ] &&
        echo "  Upstream project url: $upstream"
    return 1
}

checkdep cloud-localds cloud-image-utils
checkdep virt-install virtinst
checkdep qemu-img qemu-kvm
checkdep virsh libvirt-bin
checkdep wget wget

if [ -n "$MISSING" ]; then
    echo
    [ -n "${FOUND}" ] && echo "found: ${FOUND}"
    echo "install missing deps with:"
    echo "  apt-get update && apt-get install -y ${MISSING}"
else
    echo "All dependencies look good. (${FOUND})"
fi

echo "========= Step 2: Download ubuntu 18.08LTS cloud image if not here ..."
IMG_PATH="/var/lib/libvirt/images"
BASE_IMG="$IMG_PATH/bionic-server-cloudimg-amd64.img"

if [ ! -f "$BASE_IMG" ]; then
    wget "${_PROTO:-https}://cloud-images.ubuntu.com/bionic/current/bionic-server-cloudimg-amd64.img"
    # Resize the disk to 20G
    qemu-img resize bionic-server-cloudimg-amd64.img 20G
    # Make sure it's qcow2 format before put into location as base image
    qemu-img convert -f qcow2 -O qcow2 bionic-server-cloudimg-amd64.img $BASE_IMG
fi

echo "========= Step 3: Clean up old deployment ..."
vms=($(virsh list | grep znode | awk '{ print $2 }'))
for vm in "${vms[@]}"
do
    virsh destroy $vm
    virsh undefine $vm --remove-all-storage
done

echo "========= Step 4: Create $CNT VM disk images based on base image ..."
for ((i=0; i<CNT; i++)); do
    qemu-img create -f qcow2 -F qcow2 -b $BASE_IMG $IMG_PATH/znode$i.img
done

echo "========= Step 5: Create $CNT VM Cloudinit configuration image ..."
rm -f user_data_znode*
rm -f network_config_znode*
for ((i=0; i<CNT; i++)); do
    cat > user_data_znode$i <<EOF
#cloud-config
hostname: znode${i}
sudo: ALL=(ALL) NOPASSWD:ALL
shell: /bin/bash
password: Lab12345
ssh_pwauth: True
disable_root: false
chpasswd: { expire: False }
packages:
  - qemu-guest-agent
  - rpcbind
EOF

    let "ip = 190 + $i"
    cat > network_config_znode$i <<EOF
---
version: 2
ethernets:
    ens2:
        addresses:
          - 10.213.43.${ip}/24
        gateway4: 10.213.43.1
        nameservers:
          addresses:
            - 10.208.115.34
            - 10.208.115.35
    ens3:
        mtu: 9000
        addresses:
          - 192.168.100.${ip}/24
    ens4:
        mtu: 9000
        addresses:
          - 192.168.20.${ip}/24
EOF

cloud-localds -N network_config_znode$i $IMG_PATH/znode$i.iso user_data_znode$i
done

echo "========= Step 6: Deploy ZGC node VMs ..."
for ((k=0; k<CNT; k++)); do
    virt-install \
    --vcpus 4 \
    --memory 8192 \
    --os-type linux \
    --os-variant ubuntu18.04 \
    --virt-type kvm \
    --graphics none \
    --noautoconsole \
    --network bridge=br-mgmt,model=virtio \
    --network network=zgc-network,model=virtio \
    --network bridge=br-tenant,model=virtio \
    --name znode$k \
    --disk /var/lib/libvirt/images/znode$k.img,device=disk,bus=virtio \
    --disk /var/lib/libvirt/images/znode$k.iso,device=cdrom \
    --import
done
