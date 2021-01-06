# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: EndPoint table for NBI API
#
import os

from flask import (
    Blueprint, jsonify, request
)

from project.api.models import EP
from project import db


eps_blueprint = Blueprint('eps', __name__)


@eps_blueprint.route('/eps', methods=['GET'])
def all_eps():
    response_object = [ep.to_json() for ep in EP.query.all()]
    return jsonify(response_object)


@eps_blueprint.route('/eps/ping', methods=['GET'])
def ping_eps():
    return jsonify({
        'status': 'success',
        'message': 'pong!',
        'container_id': os.uname()[1]
    })


@eps_blueprint.route('/eps/<ep_id>', methods=['GET'])
def single_ep(ep_id):
    ep = EP.query.filter_by(ep_id=ep_id).first()
    response_object = ep.to_json()
    return jsonify(response_object)


if __name__ == '__main__':
    app.run()
