#!/usr/bin/env bash
#
# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang  <@liangbin>
#
# Summary: script to initialize deployment environment
#

# set -x

# ask if script runs without sudo or root priveleges
if [ "$(whoami)" == "root" ]; then
    echo " you need to run this script as normal user"
    exit 1
fi

# Save current location
myloc=$(pwd)

# Get full path of this script no matter where it's placed and invoked
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# print a pretty header
echo
echo " +---------------------------------------------------------+"
echo " |            Setup auto deployment environment            |"
echo " |                                                         |"
echo " | Attention:                                              |"
echo " |     don't deploy concurrently on same target cluster!!! |"
echo " +---------------------------------------------------------+"
echo
read -p " Set Vault password to keep your secrets: " vaultPass
echo "" 
read -p " Enter your AWS Access Key ID: " -i "123" VAULT_AWS_ACCESS_KEY_ID
read -p " Enter your AWS Secret Access Key: " -i "None" VAULT_AWS_SECRET_ACCESS_KEY
read -p " Enter your local host sudo password: " -i "None" VAULT_LOCAL_ENABLE_PSWD
read -p " Enter your Target hosts sudo password: " -i "None" VAULT_TARGET_ENABLE_PSWD

# Save vault password locally
echo "${vaultPass}" > $ROOT/playbooks/inventories/vars/.vault_pass

# Create a vault file to keep secrets
echo "
---
VAULT_AWS_ACCESS_KEY_ID: \"$VAULT_AWS_ACCESS_KEY_ID\"
VAULT_AWS_SECRET_ACCESS_KEY: \"$VAULT_AWS_SECRET_ACCESS_KEY\"
VAULT_LOCAL_ENABLE_PSWD: \"$VAULT_LOCAL_ENABLE_PSWD\"
VAULT_TARGET_ENABLE_PSWD: \"$VAULT_TARGET_ENABLE_PSWD\"
" > $ROOT/playbooks/inventories/vars/.vault.yml
ansible-vault encrypt $ROOT/playbooks/inventories/vars/.vault.yml --vault-password-file $ROOT/playbooks/inventories/vars/.vault_pass

# Touch .env_changed to indicate work environment changes, 
touch $ROOT/.env_changed
