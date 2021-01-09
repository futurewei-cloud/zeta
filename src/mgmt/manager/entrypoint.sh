#!/bin/sh
# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: zeta-manager container entry point
#

echo "Waiting for postgres..."

while ! nc -z postgres 5432; do
  sleep 1
done
sleep 10

echo "PostgreSQL started"

# Trigger Ready Probe
touch /tmp/healthy

gunicorn -b 0.0.0.0:5000 manage:app --error-logfile - --access-logfile - --log-level info
