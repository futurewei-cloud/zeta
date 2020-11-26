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
from functools import reduce
from kubernetes.client.rest import ApiException
from project.api.models import Node
from project import db
from .models import Zgc
from kubernetes import client, config
import json
from .vpcs import getGWsFromIpRange, MAC_PRIVATE, int_to_mac

logger = logging.getLogger()
config.load_incluster_config()
obj_api = client.CustomObjectsApi()

nodes_blueprint = Blueprint('nodes', __name__)

def get_mac_from_ip(ip_string):
    start= reduce(lambda a,b: a<<8 | b, map(int, ip_string.split('.')))
    mac = int_to_mac(start | MAC_PRIVATE)
    return mac

def update_droplet(droplet):
    try:
        update_response = obj_api.replace_namespaced_custom_object(group='zeta.com',
                                                                   version='v1',
                                                                   plural='droplets',
                                                                   namespace='default', 
                                                                   name=droplet['metadata']['name'],
                                                                   body=droplet)
        logger.info('Response for update droplet: {}'.format(update_response))
    except ApiException as e:
        logger.error('Exception when updating exsiting droplet: {}'.format(e))

def delete_droplet(name):
    try:
        delete_response = obj_api.delete_namespaced_custom_object(group='zeta.com',
                                                                  version='v1',
                                                                  namespace='default',
                                                                  plural='droplets', 
                                                                  name=name,
                                                                  body=client.V1DeleteOptions(),
                                                                  propagation_policy="Orphan")
        logger.info('Response for delete droplet: {}'.format(update_response))
    except ApiException as e:
        logger.error('Exception when deleting droplet: {}\n{}'.format(name, e))

def create_droplet(name, ip, mac, itf, network, zgc_id):
    droplet_body = dict()
    meta_data = dict()
    meta_data['name'] = name
    meta_data['labels'] = dict()
    meta_data['labels']['zgc_id'] = zgc_id
    meta_data['labels']['network'] = network
    spec = dict()
    spec['ip'] = ip
    spec['mac'] = mac
    spec['status'] = 'Init'
    spec['itf'] = itf
    spec['network'] = network
    spec['zgc_id'] = zgc_id
    droplet_body['metadata'] = meta_data
    droplet_body['spec'] = spec
    droplet_body['apiVersion'] = 'zeta.com/v1'
    droplet_body['kind'] = 'Droplet'
    try:
        create_response = obj_api.create_namespaced_custom_object(group='zeta.com',
                                                                  version='v1',
                                                                  namespace='default',
                                                                  plural='droplets', 
                                                                  body=droplet_body)
        logger.info('Response for create droplet: {}'.format(update_response))
    except ApiException as e:
        logger.error('Exception when Creating droplets: {}'.format(e))

@nodes_blueprint.route('/nodes', methods=['GET', 'POST'])
def all_nodes():
    if request.method == 'POST':
        post_data = request.get_json()
        post_data['node_id'] = str(uuid.uuid4())
        response_object = post_data

        # Try to get the existing droplets        
        all_droplets_in_zgc = obj_api.list_cluster_custom_object(group='zeta.com',
                                                                 version='v1',
                                                                 plural='droplets',
                                                                 label_selector='zgc_id='+post_data['zgc_id']+',network=tenant'
                                                                )['items']
        zgc_ip_list = post_data['ip_control'].split('.')
        zgc_ip_list[0] = '10'
        zgc_ip_list[1] = '0'
        zgc_ip = '.'.join(zgc_ip_list)
        zgc_mac = post_data['mac_zgc']

        if len(all_droplets_in_zgc)>0:
            total_ip = 0
            droplet_that_can_give_ip_mac = []
            for droplet in all_droplets_in_zgc:
                droplet_spec = droplet['spec']
                droplet_ip_list = droplet_spec['ip']
                droplet_mac_list = droplet_spec['mac']
                total_ip = total_ip + len(droplet_ip_list)
                if len(droplet_ip_list) > 1 and len(droplet_mac_list) > 1:
                    droplet_that_can_give_ip_mac.append(droplet)
            number_of_droplets = len(all_droplets_in_zgc)
            number_of_ip_new_droplet_gets = total_ip // (number_of_droplets + 1) 
            ip_for_new_droplet = []
            if number_of_ip_new_droplet_gets > 0 : # each droplet should have 1 or more ip, assign ip / mac from exiting droplets
                current_length_of_ip_list = len(ip_for_new_droplet)
                while len(ip_for_new_droplet) < number_of_ip_new_droplet_gets:
                    for droplet in droplet_that_can_give_ip_mac:
                        droplet_spec = droplet['spec']
                        droplet_ip_list = droplet_spec['ip']
                        if len(droplet_ip_list) > 1:
                            popped_ip = droplet_ip_list.pop()
                            ip_for_new_droplet.append(popped_ip)
                            droplet_mac_list = droplet_spec['mac']
                            mac_from_ip =  get_mac_from_ip(popped_ip)
                            if mac_from_ip in droplet_mac_list:
                                droplet_mac_list.remove(mac_from_ip)
                            else:
                                droplet_mac_list.pop()
                            droplet['spec']['ip'] = droplet_ip_list
                            droplet['spec']['mac'] = droplet_mac_list
                    if current_length_of_ip_list == len(ip_for_new_droplet):
                        logger.info('Breaking the while loop as there is no new ip added')
                        break
                    else:
                        current_length_of_ip_list = len(ip_for_new_droplet)
                macs_for_new_droplet = [get_mac_from_ip(ip) for ip in ip_for_new_droplet]

                for droplet in droplet_that_can_give_ip_mac:
                    update_droplet(droplet)

                create_droplet(name='droplet-' +'tenant-' + post_data['name'].replace('_', "-").lower(), 
                               ip=ip_for_new_droplet, 
                               mac=macs_for_new_droplet,
                               itf=post_data['inf_tenant'], 
                               network='tenant', 
                               zgc_id=post_data['zgc_id'])

                create_droplet(name='droplet-' + 'zgc-' + post_data['name'].replace('_', "-").lower(), 
                               ip=[zgc_ip], 
                               mac=[zgc_mac],
                               itf=post_data['inf_zgc'], 
                               network='zgc', 
                               zgc_id=post_data['zgc_id'])

                logger.info('Finished updating and adding droplets.')
            else:
                logger.error('Not enough ip to assign for the new droplet.')
        else:
            logger.info("First nodes in the ZGC cluster, it gets all the IPs")
            zgc = Zgc.query.filter_by(zgc_id=post_data['zgc_id']).first()
            if zgc is not None:
                gws = getGWsFromIpRange(zgc.ip_start, zgc.ip_end)
                create_droplet(name='droplet-' +'tenant-' + post_data['name'].replace('_', "-").lower(), 
                               ip=[gw['ip'] for gw in gws], 
                               mac=[gw['mac'] for gw in gws],
                               itf=post_data['inf_tenant'], 
                               network='tenant', 
                               zgc_id=post_data['zgc_id'])
                create_droplet(name='droplet-' +'zgc-' + post_data['name'].replace('_', "-").lower(), 
                               ip=[zgc_ip], 
                               mac=[zgc_mac],
                               itf=post_data['inf_zgc'], 
                               network='zgc', 
                               zgc_id=post_data['zgc_id'])
            else:
                logger.info("There's no zgc with zgc_id: {} in the database!".format(post_data['zgc_id']))
        # commit change to data at last
        db.session.add(Node(**post_data))
        db.session.commit()
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
        node.id_control = post_data.get('id_control')
        node.pwd_control = post_data.get('pwd_control')
        node.inf_tenant = post_data.get('inf_tenant')
        node.mac_tenant = post_data.get('mac_tenant')
        node.inf_zgc = post_data.get('inf_zgc')
        node.mac_zgc = post_data.get('mac_zgc')
        db.session.commit()
        response_object = node.to_json()
    elif request.method == 'DELETE':
        post_data = request.get_json()
        node_name = node.name
        zgc_id = node.zgc_id
        all_droplets_in_zgc = obj_api.list_cluster_custom_object(group='zeta.com',
                                                                 version='v1',
                                                                 plural='droplets',
                                                                 label_selector='zgc_id='+zgc_id+',network=tenant'
                                                                )['items']
        droplet_to_remove = None

        tenant_droplet_name = 'droplet-tenant-'+node_name.replace('_', "-").lower()
        zgc_droplet_name = 'droplet-zgc-'+node_name.replace('_', "-").lower()

        for droplet in all_droplets_in_zgc:
            if droplet['metadata']['name'] == tenant_droplet_name:
                droplet_to_remove = droplet
                all_droplets_in_zgc.remove(droplet)
                break

        if droplet_to_remove is None:
            logger.error('Cannot find that droplet with name: {}, thus not deleting anything'.format(tenant_droplet_name))
            return jsonify({})
        else:
            ip_to_assign = droplet_to_remove['spec']['ip']

            ip_amount = len(ip_to_assign)

            number_of_droplets_to_assign = len(all_droplets_in_zgc)

            modified_droplets = []
            for index in range(ip_amount):
                ip = ip_to_assign[index]
                mac = get_mac_from_ip(ip)
                all_droplets_in_zgc[index % number_of_droplets_to_assign]['spec']['ip'].append(ip)
                all_droplets_in_zgc[index % number_of_droplets_to_assign]['spec']['mac'].append(mac)
                if all_droplets_in_zgc[index % number_of_droplets_to_assign] not in modified_droplets: 
                    modified_droplets.append(all_droplets_in_zgc[index % number_of_droplets_to_assign])

            delete_droplet(name=tenant_droplet_name)
            delete_droplet(name=zgc_droplet_name)

            for droplet in modified_droplets:
                update_droplet(droplet)

        db.session.delete(node)
        db.session.commit()
        response_object = {}
    return jsonify(response_object)


if __name__ == '__main__':
    app.run()
