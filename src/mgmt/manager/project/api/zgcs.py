# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: ZGC table for NBI API
#

import os
import uuid

from flask import (
    Blueprint, jsonify, request
)

from project.api import settings
from project.api.models import Zgc
from project import db
import time
import logging

logger = logging.getLogger()
zgcs_blueprint = Blueprint('zgcs', __name__)


@zgcs_blueprint.route('/zgcs', methods=['GET', 'POST'])
def all_zgcs():
    status_code = 200
    if request.method == 'POST':
        logger.debug('Start to make a ZGC',)
        start_time = time.time()
        post_data = request.get_json()
        post_data['zgc_id'] = str(uuid.uuid4())
        db.session.add(Zgc(**post_data))
        db.session.commit()
        response_object = post_data

        if not bool(settings.activeZgc):
            settings.activeZgc["zgc_id"] = post_data["zgc_id"]
            settings.activeZgc["ip_start"] = post_data["ip_start"]
            settings.activeZgc["ip_end"] = post_data["ip_end"]
            settings.activeZgc["port_ibo"] = post_data["port_ibo"]
        end_time = time.time()
        logger.debug(f'Zeta took {end_time - start_time} seconds to make a ZGC')
        status_code = 201
    else:
        response_object = [zgc.to_json() for zgc in Zgc.query.all()]
    return jsonify(response_object), status_code


@zgcs_blueprint.route('/zgcs/ping', methods=['GET'])
def ping_zgcs():
    return jsonify({
        'status': 'success',
        'message': 'pong!',
        'container_id': os.uname()[1]
    })


@zgcs_blueprint.route('/zgcs/<zgc_id>', methods=['GET', 'PUT', 'DELETE'])
def single_zgc(zgc_id):
    zgc = Zgc.query.filter_by(zgc_id=zgc_id).first()
    status_code = 200
    if request.method == 'GET':
        response_object = zgc.to_json()
    elif request.method == 'PUT':
        post_data = request.get_json()
        zgc.description = post_data.get('description')
        zgc.ip_start = post_data.get('ip_start')
        zgc.ip_end = post_data.get('ip_end')
        zgc.port_ibo = post_data.get('port_ibo')
        zgc.overlay_type = post_data.get('overlay_type')
        db.session.commit()
        response_object = zgc.to_json()
    elif request.method == 'DELETE':
        db.session.delete(zgc)
        db.session.commit()
        response_object = {}
        if 'zgc_id' in settings.activeZgc and settings.activeZgc["zgc_id"] == zgc_id:
            zgc = Zgc.query.first()
            if zgc is not None:
                settings.activeZgc["zgc_id"] = zgc.zgc_id
                settings.activeZgc["ip_start"] = zgc.ip_start
                settings.activeZgc["ip_end"] = zgc.ip_end
                settings.activeZgc["port_ibo"] = zgc.port_ibo
            else:
                settings.activeZgc.clear()
        status_code = 204
    return jsonify(response_object), status_code


if __name__ == '__main__':
    app.run()
