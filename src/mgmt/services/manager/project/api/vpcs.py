# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: VPC table for NBI API
#


import os
from functools import reduce

from flask import (
    Blueprint, jsonify, request
)

from project.api import settings
from project.api.models import Vpc
from project import db
from project.api.utils import extendVpcResp


vpcs_blueprint = Blueprint('vpcs', __name__)



@vpcs_blueprint.route('/vpcs', methods=['GET', 'POST'])
def all_vpcs():
    if request.method == 'POST':
        post_data = request.get_json()
        vpc = Vpc(zgc_id = settings.activeZgc["zgc_id"], **post_data)
        db.session.add(vpc)
        db.session.commit()

        response_object = extendVpcResp(vpc)
    else:
        response_object = []
        for vpc in Vpc.query.all():
            resp = extendVpcResp(vpc)
            response_object.append(resp)

    return jsonify(response_object)


@vpcs_blueprint.route('/vpcs/ping', methods=['GET'])
def ping_vpcs():
    return jsonify({
        'status': 'success',
        'message': 'pong!',
        'container_id': os.uname()[1]
    })


@vpcs_blueprint.route('/vpcs/<vpc_id>', methods=['GET', 'PUT', 'DELETE'])
def single_vpc(vpc_id):
    vpc = Vpc.query.filter_by(vpc_id=vpc_id).first()
    if request.method == 'GET':
        response_object = extendVpcResp(vpc)
    elif request.method == 'PUT':
        post_data = request.get_json()
        vpc.vni = post_data.get('vni')
        db.session.commit()
        response_object = vpc.to_json()
    elif request.method == 'DELETE':
        db.session.delete(vpc)
        db.session.commit()
        response_object = {}
    return jsonify(response_object)


if __name__ == '__main__':
    app.run()
