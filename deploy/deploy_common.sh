#!/usr/bin/env bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang  <@liangbin>
#
# Summary: script to setup k8s cluster and deploy Zeta services
#

#set -x

# ask if script runs without sudo or root priveleges
if [ "$(whoami)" == "root" ]; then
    echo " you need to run this script as normal user"
    exit 1
fi

# Save current location
myloc=$(pwd)
# Get full path of this script no matter where it's placed and invoked
MY_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [[ ! -f $MY_PATH/playbooks/inventories/vars/.vault.yml ]]; then
  echo -n " Can't find your vault, run $MY_PATH/env_setuo.sh as normal user first!"
  exit 1
fi

# Check inline arguments, otherwise interactive
if [ "$#" -eq 0 ] || [ "$1" == "" ] ; then
  # print a pretty header
  echo " +---------------------------------------------------+"
  echo " |            Interactive deployment                 |"
  echo " +---------------------------------------------------+"
  echo
  read -ep " Enter site to deploy to [kind|aws|lab]: " -i "kind" site
  while true; do
    echo " Which deployment favlor:"
    echo
    echo "  [d] For development deployment"
    echo "  [p] For production publishment and deployment"
    echo "  [u] For production deployment without publish"
    echo "  [r] For Removal of current deployment (virtual sites only)"
    echo "  [c] Cancel"
    echo
    read -p " please enter your preference: [d|p|u|r]: " opt
    case $opt in
      [d]* )  
            stage="development"
            break
            ;;
      [p]* )
            stage="production"
            break
            ;;
      [u]* )
            stage="user"
            break
            ;;
      [r]* )
            stage="undeploy"
            break
            ;;
      [c]* )  exit 0;;
        * ) echo " please pick one from [d], [p] [u] or [r]";;
    esac
  done
else
  dpur_opt=false

  while getopts "d:p:u:r:" opt; do
    case $opt in
      d)
        stage="development"
        site="$OPTARG"
        dpur_opt=true
        ;;
      p)
        stage="production"
        site="$OPTARG"
        dpur_opt=true
        ;;
      u)
        stage="user"
        site="$OPTARG"
        dpur_opt=true
        ;;
      r)
        stage="undeploy"
        site="$OPTARG"
        dpur_opt=true
        ;;
      \?)
        echo "Invalid operation: -$opt $OPTARG" >&2
        echo "-d <kind|aws|lab>   For development deployment" >&2
        echo "-p <kind|aws|lab>   For production artifacts publishment and deployment" >&2
        echo "-u <kind|aws|lab>   For deployment with production artifacts" >&2
        echo "-r <kind|aws|lab>   For Removal of current deployment" >&2
        ;;
    esac
  done

  if ! $dpur_opt 
  then
    echo " must enter operation!" >&2
    exit 1
  fi
fi

if [[ ! -f $MY_PATH/playbooks/inventories/vars/site_$site.yml ]]; then
    echo "Please add site specific variables to $MY_PATH/playbooks/inventories/vars/site_$site.yml first" >&2
    exit 1
fi

# Choose right registry based on stage and k8s cluster type
k8s_type="$(grep k8s_type $MY_PATH/playbooks/inventories/vars/site_$site.yml | awk '{ print $2 }' | sed -e 's/^"//' -e 's/"$//')"
reg="localhost:5000"
if [[ "$stage" == "user" || "$stage" == "production" ]]; then
    reg="fwnetworking"
elif [[ "$k8s_type" == "microk8s" ]]; then
    reg="localhost:32000"
fi

# Capture detailed script output for trouble shooting
ANSIBLE_DBG_LOG="/tmp/ansible_debug.log"
echo "" > $ANSIBLE_DBG_LOG
echo "Detailed deployment output are available at $ANSIBLE_DBG_LOG and /var/log/ansible.log"

## compose play options
PLAY_CMD="ansible-playbook ${ANSIBLE_VERBOSE} \
-i inventories/hosts_${site} \
-e \"site=${site} stage=${stage} reg=${reg} DBG_LOG=$ANSIBLE_DBG_LOG\" \
-e \"@inventories/vars/.vault.yml\" \
-e \"@inventories/vars/site_${site}.yml\" \
"
