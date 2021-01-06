# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: Host table for NBI API
#

import os

from flask import (
    Blueprint, jsonify, request
)

from project.api.models import Host
from project import db


hosts_blueprint = Blueprint('hosts', __name__)


@hosts_blueprint.route('/hosts', methods=['GET'])
def all_hosts():
    response_object = [host.to_json() for host in Host.query.all()]
    return jsonify(response_object)


@hosts_blueprint.route('/hosts/ping', methods=['GET'])
def ping_hosts():
    return jsonify({
        'status': 'success',
        'message': 'pong!',
        'container_id': os.uname()[1]
    })


@hosts_blueprint.route('/hosts/<host_id>', methods=['GET'])
def single_host(host_id):
    host = Host.query.filter_by(host_id=host_id).first()
    response_object = host.to_json()
    return jsonify(response_object)


if __name__ == '__main__':
    app.run()
