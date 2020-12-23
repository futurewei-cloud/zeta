# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: Port table for NBI API
#


import os
import uuid

from flask import (
    Blueprint, jsonify, request
)

from project.api.models import Port
from project.api.models import Host
from project.api.models import EP
from project import db
import time
import logging

logger = logging.getLogger('gunicorn.error')

ports_blueprint = Blueprint('ports', __name__)


@ports_blueprint.route('/ports', methods=['GET', 'POST'])
def all_ports():
    if request.method == 'POST':
        portList = request.get_json()
        amount_of_ports = len(portList)
        logger.debug(f'Start to make {amount_of_ports} ports.')
        start_time = time.time()
        # need to add dicts, not Port/Host
        ports_to_add = []
        hosts_to_add = []
        for post_data in portList:
            port = {
                'port_id': post_data.get('port_id'),
                'mac_port': post_data.get('mac_port'),
                'vpc_id': post_data.get('vpc_id'),
                'host_id': '',
                'eps': []
            }

            ip_node_to_find = post_data.get('ip_node')
            logger.debug(f'Need to find ip_node: [{ip_node_to_find}]')
            host = Host.query.filter_by(ip_node=ip_node_to_find).first()
            if (host is None) and (ip_node_to_find not in [to_add_host['ip_node'] for to_add_host in hosts_to_add]):
                logger.debug(f'Cannot find ip_node [{ip_node_to_find}] in either the db or the hosts to add, create a new host to add')
                host_to_add = {'mac_node': post_data.get('mac_node'),
                    'ip_node' : ip_node_to_find,
                    'host_id' : str(uuid.uuid4())}
                port['host_id'] = host_to_add['host_id']
                logger.debug(f'Created this host_to_add: {host_to_add}')
                hosts_to_add.append(host_to_add)
            elif (host is not None):
                logger.debug(f'In DB, found host with host_id: [{host.host_id}]')
                port['host_id'] = host.host_id
            elif (ip_node_to_find in [to_add_host['host_id'] for to_add_host in hosts_to_add]):
                logger.debug(f'Host_id: [{ip_node_to_find}] should be found in hosts_to_add')
                for host_to_add in hosts_to_add:
                    if host_to_add['ip_node'] == ip_node_to_find:
                        logger.debug(f'In hosts_to_add, found host with host_id: [{ip_node_to_find}]')
                        port['host_id'] = host_to_add['host_id']
                        break
                logger.debug(f'Host_id: [{ip_node_to_find}] should be found in hosts_to_add, but it is not found after the while loop, need to investigate')
            else:
                logger.debug(f'Something weird happend for querying host and looking up in the hosts_to_add')
                logger.debug(f'host in query: {host}')
                logger.debug(f'hosts_to_add: ')
                for host_to_add in hosts_to_add:
                    logger.debug(f'One host_to_add: {host_to_add}')
            for ip in post_data['ips_port']:
                ep_to_add = {
                                'ip': ip['ip'],
                                'vip': ip['vip'],
                                'port_id': post_data['port_id']
                            }
                port['eps'].append(ep_to_add)
                logger.debug(f'Added this ep to the port: {ep_to_add}')
            logger.debug(f'Adding this port object to the list: \n{port}')
            ports_to_add.append(port)
        db.session.bulk_insert_mappings(Host, hosts_to_add)
        db.session.commit()
        db.session.bulk_insert_mappings(Port, ports_to_add)
        db.session.commit()
        response_object = portList
        end_time = time.time()
        logger.debug(f'Zeta took {end_time - start_time} seconds to make {amount_of_ports} ports')
    else:
        response_object = [port.to_json() for port in Port.query.all()]
    return jsonify(response_object)


@ports_blueprint.route('/ports/ping', methods=['GET'])
def ping_ports():
    return jsonify({
        'status': 'success',
        'message': 'pong!',
        'container_id': os.uname()[1]
    })


@ports_blueprint.route('/ports/<port_id>', methods=['GET', 'PUT', 'DELETE'])
def single_port(port_id):
    port = Port.query.filter_by(port_id=port_id).first()
    if request.method == 'GET':
        response_object = port.to_json()
    elif request.method == 'PUT':
        post_data = request.get_json()
        port.mac_port = post_data.get('mac_port')
        db.session.commit()
        response_object = port.to_json()
    elif request.method == 'DELETE':
        db.session.delete(port)
        db.session.commit()
        response_object = {}
    return jsonify(response_object)


if __name__ == '__main__':
    app.run()
