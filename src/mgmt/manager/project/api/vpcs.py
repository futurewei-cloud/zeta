# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: VPC table for NBI API
#


import os

from flask import (
    Blueprint, jsonify, request
)

from project.api import settings
from project.api.models import Vpc
from project import db
from project.api.utils import getGWsFromIpRange
import time
import logging

logger = logging.getLogger()

vpcs_blueprint = Blueprint('vpcs', __name__)

def extendVpcResp(vpc):
    respond = vpc.to_json()
    respond['port_ibo'] = settings.activeZgc["port_ibo"]
    respond['gws'] = getGWsFromIpRange(settings.activeZgc["ip_start"], settings.activeZgc["ip_end"])
    return respond

@vpcs_blueprint.route('/vpcs', methods=['GET', 'POST'])
def all_vpcs():
    if request.method == 'POST':
        logger.debug('Start to make a VPC.')
        start_time = time.time()
        post_data = request.get_json()
        vpc = Vpc(zgc_id = settings.activeZgc["zgc_id"], **post_data)
        db.session.add(vpc)
        db.session.commit()
        settings.vnis[post_data.get('vpc_id')] = post_data.get('vni')

        response_object = extendVpcResp(vpc)
        end_time = time.time()
        logger.debug(f'Zeta took {end_time - start_time} seconds to make a VPC')
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
