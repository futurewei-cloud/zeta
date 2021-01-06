# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: Zeta Network node table for NBI API
#


import os
import uuid
import logging
import json
import operator
import time
from flask import (
    Blueprint, jsonify, request
)
from kubernetes.client.rest import ApiException
from kubernetes import client, config
from project import db
from project.api.models import Node
from project.api.models import Zgc
from project.api.utils import getGWsFromIpRange, get_mac_from_ip
from project.api.settings import activeZgc, zgc_cidr_range, node_ips
from common.rpc import TrnRpc

logger = logging.getLogger('gunicorn.error')

config.load_incluster_config()
obj_api = client.CustomObjectsApi()
nodes_blueprint = Blueprint('nodes', __name__)


def update_droplet(droplet):
    try:
        update_response = obj_api.replace_namespaced_custom_object(group='zeta.com',
                                                                   version='v1',
                                                                   plural='droplets',
                                                                   namespace='default', 
                                                                   name=droplet['metadata']['name'],
                                                                   body=droplet)
        logger.info('Response for update droplet: {}'.format(update_response))
        droplet_update_itf_config(droplet['metadata']['labels']['node_ip'], droplet)
    except ApiException as e:
        logger.error('Exception when updating existing droplet: {}'.format(e))

def delete_droplet(name):
    try:
        delete_response = obj_api.delete_namespaced_custom_object(group='zeta.com',
                                                                  version='v1',
                                                                  namespace='default',
                                                                  plural='droplets', 
                                                                  name=name,
                                                                  body=client.V1DeleteOptions(),
                                                                  propagation_policy="Orphan")
        logger.info('Response for delete droplet: {}'.format(delete_response))
    except ApiException as e:
        logger.error('Exception when deleting droplet: {}\n{}'.format(name, e))

def create_droplet(name, ip, mac, itf, node_ip, network, zgc_id):
    droplet_body = dict()
    meta_data = dict()
    meta_data['name'] = name
    meta_data['labels'] = dict()
    meta_data['labels']['zgc_id'] = zgc_id
    meta_data['labels']['node_ip'] = node_ip
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
        logger.info('Response for create droplet: {}'.format(create_response))
        droplet_update_itf_config(node_ip, droplet_body)
    except ApiException as e:
        logger.error('Exception when Creating droplets: {}'.format(e))

def droplet_update_itf_config(node_ip, droplet):
    entrances = []
    ips = droplet.get('spec', {}).get('ip')
    macs = droplet.get('spec', {}).get('mac')

    for idx, ip in enumerate(ips):
        entrance = {'ip': ip, 'mac': macs[idx]}
        entrances.append(entrance)
    itf_conf = {
        'interface': droplet['spec']['itf'],
        'num_entrances': len(ips),
        'entrances': entrances
    }
    logger.info('Sending RPC: {}-{}'.format(node_ip, itf_conf))
    rpc = TrnRpc(node_ip)
    rpc.update_droplet(itf_conf)
    del rpc

def node_load_transit_xdp(ip, inf_tenant, inf_zgc):
    node_ips[ip] = ip
    logger.info('Sending RPC: {}-{} {}'.format(ip, inf_tenant, inf_zgc))
    rpc = TrnRpc(ip)
    rpc.load_transit_xdp(inf_tenant, inf_zgc, int(activeZgc["port_ibo"]))
    del rpc

def node_unload_transit_xdp(ip, itf_tenant, itf_zgc):
    node_ips.pop(ip, None)
    rpc = TrnRpc(ip)
    rpc.unload_transit_xdp(ip, itf_tenant, itf_zgc)
    del rpc
    
@nodes_blueprint.route('/nodes', methods=['GET', 'POST'])
def all_nodes():
    status_code = 200
    if request.method == 'POST':
        logger.debug('Start to make one node, and two droplets for this node.')
        start_time = time.time()
        post_data = request.get_json()
        post_data['node_id'] = str(uuid.uuid4())

        node_load_transit_xdp(post_data['ip_control'], post_data['inf_tenant'], post_data['inf_zgc'])

        # Try to get the existing droplets        
        all_droplets_in_zgc = obj_api.list_cluster_custom_object(group='zeta.com',
                                                                 version='v1',
                                                                 plural='droplets',
                                                                 label_selector='zgc_id='+post_data['zgc_id']+',network=tenant'
                                                                )['items']
        zgc_ip_list = post_data['ip_control'].split('.')
        
        ip_range = zgc_cidr_range.split('/')
        cidr_list = ip_range[0].split('.')

        # Only zgc_cidr postfix 8, 16 and 24 are supported
        for i in range(int(ip_range[1])//8):
            zgc_ip_list[i] = cidr_list[i]

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
            modified_droplets = dict()
            if number_of_ip_new_droplet_gets > 0 : # each droplet should have 1 or more ip, assign ip / mac from existing droplets
                current_length_of_ip_list = len(ip_for_new_droplet)
                # sort these droplets in descending order, so droplet with the most IPs will be in the front.
                droplet_that_can_give_ip_mac.sort(key=lambda x : len(x['spec']['ip']), reverse=True)
                while len(ip_for_new_droplet) < number_of_ip_new_droplet_gets:
                    for droplet in droplet_that_can_give_ip_mac:
                        droplet_spec = droplet['spec']
                        droplet_ip_list = droplet_spec['ip']
                        if len(droplet_ip_list) > 1:
                            droplet_name = droplet['metadata']['name']
                            if droplet_name not in modified_droplets:
                                modified_droplets[droplet_name] = droplet
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

                for droplet_name in modified_droplets:
                    update_droplet(modified_droplets[droplet_name])

                create_droplet(name='droplet-tenant-' + post_data['name'].replace('_', "-").lower(), 
                               ip=ip_for_new_droplet, 
                               mac=macs_for_new_droplet,
                               itf=post_data['inf_tenant'], 
                               node_ip= post_data['ip_control'],
                               network='tenant', 
                               zgc_id=post_data['zgc_id'])

                create_droplet(name='droplet-zgc-' + post_data['name'].replace('_', "-").lower(), 
                               ip=[zgc_ip], 
                               mac=[zgc_mac],
                               itf=post_data['inf_zgc'], 
                               node_ip= post_data['ip_control'],
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
                create_droplet(name='droplet-tenant-' + post_data['name'].replace('_', "-").lower(), 
                               ip=[gw['ip'] for gw in gws], 
                               mac=[gw['mac'] for gw in gws],
                               itf=post_data['inf_tenant'],
                               node_ip= post_data['ip_control'],
                               network='tenant', 
                               zgc_id=post_data['zgc_id'])
                create_droplet(name='droplet-' +'zgc-' + post_data['name'].replace('_', "-").lower(), 
                               ip=[zgc_ip], 
                               mac=[zgc_mac],
                               itf=post_data['inf_zgc'], 
                               node_ip= post_data['ip_control'],
                               network='zgc', 
                               zgc_id=post_data['zgc_id'])
            else:
                logger.error("There's no zgc with zgc_id: {} in the database!".format(post_data['zgc_id']))
                return jsonify({'error':'No such zgc'})
        # commit change to data at last
        db.session.add(Node(**post_data))
        db.session.commit()

        response_object = post_data
        end_time = time.time()
        logger.debug(f'Zeta took {end_time - start_time} seconds to make a node and its two droplets.')
        status_code = 201
    else:
        response_object = [node.to_json() for node in Node.query.all()]
    return jsonify(response_object), status_code

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
    status_code = 200
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
            return jsonify({'error': 'Droplet not found'})
        else:
            ip_to_assign = droplet_to_remove['spec']['ip']

            ip_amount = len(ip_to_assign)

            number_of_droplets_to_assign = len(all_droplets_in_zgc)

            # Sort these droplets in ascending order, so the droplet with the least IPs will be in the front.
            all_droplets_in_zgc.sort(key=lambda x : len(x['spec']['ip']), reverse=False)
            modified_droplets = dict()
            for index in range(ip_amount):
                ip = ip_to_assign[index]
                mac = get_mac_from_ip(ip)
                all_droplets_in_zgc[index % number_of_droplets_to_assign]['spec']['ip'].append(ip)
                all_droplets_in_zgc[index % number_of_droplets_to_assign]['spec']['mac'].append(mac)
                modified_droplet_name = all_droplets_in_zgc[index % number_of_droplets_to_assign]['metadata']['name']
                if modified_droplet_name not in modified_droplets:
                    modified_droplets[modified_droplet_name] = all_droplets_in_zgc[index % number_of_droplets_to_assign]

            delete_droplet(name=tenant_droplet_name)
            delete_droplet(name=zgc_droplet_name)

            for droplet_name in modified_droplets:
                update_droplet(modified_droplets[droplet_name])

        node_unload_transit_xdp(node.ip_control, node.inf_tenant, node.zgc_id)

        db.session.delete(node)
        db.session.commit()
        response_object = {}
        status_code = 204
    return jsonify(response_object), status_code


if __name__ == '__main__':
    app.run()
