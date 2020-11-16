# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: Zeta Network node table for NBI API
#


import os
import uuid

from flask import (
    Blueprint, jsonify, request
)

from project.api.models import Node
from project import db


nodes_blueprint = Blueprint('nodes', __name__)


@nodes_blueprint.route('/nodes', methods=['GET', 'POST'])
def all_nodes():
    if request.method == 'POST':
        post_data = request.get_json()
        post_data['node_id'] = str(uuid.uuid4())
        db.session.add(Node(**post_data))
        db.session.commit()
        response_object = post_data
    else:
        response_object = [node.to_json() for node in Node.query.all()]
    return jsonify(response_object)


@nodes_blueprint.route('/nodes/ping', methods=['GET'])
def ping_nodes():
    return jsonify({
        'status': 'success',
        'message': 'pong!',
        'container_id': os.uname()[1]
    })


@nodes_blueprint.route('/nodes/<node_id>', methods=['GET', 'PUT', 'DELETE'])
def single_node(node_id):
    node = Node.query.filter_by(node_id=node_id).first()
    if request.method == 'GET':
        response_object = node.to_json()
    elif request.method == 'PUT':
        post_data = request.get_json()
        node.zgc_id = post_data.get('zgc_id')
        node.description = post_data.get('description')
        node.ip_control = post_data.get('ip_control')
        node.inf_tenant = post_data.get('inf_tenant')
        node.inf_zgc = post_data.get('inf_zgc')
        db.session.commit()
        response_object = node.to_json()
    elif request.method == 'DELETE':
        db.session.delete(node)
        db.session.commit()
        response_object = {}
    return jsonify(response_object)


if __name__ == '__main__':
    app.run()
