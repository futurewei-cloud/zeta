#!/bin/bash

kernel_ver=`uname -r`
echo "Running kernel version: $kernel_ver"

if [ "$kernel_ver" != "5.10.4" ]; then
  kernel_dir="linux-5.10.4"
  mkdir -p $kernel_dir

  wget https://mizar.s3.amazonaws.com/linux-5.10.4/linux-headers-5.10.4_5.10.4-1_amd64.deb -P $kernel_dir
  wget https://mizar.s3.amazonaws.com/linux-5.10.4/linux-image-5.10.4-dbg_5.10.4-1_amd64.deb -P $kernel_dir
  wget https://mizar.s3.amazonaws.com/linux-5.10.4/linux-image-5.10.4_5.10.4-1_amd64.deb -P $kernel_dir
  wget https://mizar.s3.amazonaws.com/linux-5.10.4/linux-libc-dev_5.10.4-1_amd64.deb -P $kernel_dir

  read -p "Continue kernel update (y/n)?" choice
  case "$choice" in
    y|Y ) echo "Updating kernel";;
    n|N ) exit;;
    * ) exit;;
  esac

  sudo dpkg -i $kernel_dir/*.deb

  read -p "Reboot host (y/n)?" choice
  case "$choice" in
    y|Y ) echo "Rebooting";;
    n|N ) exit;;
    * ) exit;;
  esac

  sudo reboot
fi
