#!/bin/bash

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

function delete_pods {
    NAME=$1
    TYPE=$2
    kubectl delete $TYPE.apps/$NAME 2> /tmp/kubetctl.err
    echo -n "Waiting for ${NAME} pods to terminate."
    kubectl get pods | grep $NAME > /dev/null
    while [[ $? -eq 0 ]]; do
        echo -n "."
        sleep 1
        kubectl get pods | grep $NAME > /dev/null
    done
    echo
}

# Checks for status for given object
function get_status() {
    OBJECT=$1
    STATUS=$2
    kubectl get $OBJECT 2> /tmp/kubetctl.err | awk '
    NR==1 {
        for (i=1; i<=NF; i++) {
            f[$i] = i
        }
    }
    { print $f["STATUS"] }
    ' | grep $STATUS > /dev/null

    return $?
}

# Checks for status Provisioned of array of objects
function check_ready() {
    sum=0
    for i in $1
    do
        get_status $i ${@: -1}
        let sum+=$((sum + $?))
    done
    if [[ $sum == 0 ]]; then
        return 1
    else
        sleep 2
        echo -n "."
        return 0
    fi
}
