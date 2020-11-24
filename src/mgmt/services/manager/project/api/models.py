# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang  <@liangbin>
#
# Summary: Zeta DB schema for NBI API test
#
import datetime

from flask import current_app
from sqlalchemy.sql import func

from project import db


class Book(db.Model):

    __tablename__ = 'books'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    title = db.Column(db.String(255), nullable=False)
    author = db.Column(db.String(255), nullable=False)
    read = db.Column(db.Boolean(), default=False, nullable=False)

    def __init__(self, title, author, read):
        self.title = title
        self.author = author
        self.read = read

    def to_json(self):
        return {
            'id': self.id,
            'title': self.title,
            'author': self.author,
            'read': self.read
        }


class Zgc(db.Model):

    __tablename__ = 'zgcs'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    zgc_id = db.Column(db.String(64), unique=True, nullable=False)
    name = db.Column(db.String(128), unique=True, nullable=False)
    description = db.Column(db.String(255), nullable=True)
    ip_start = db.Column(db.String(16), nullable=False)
    ip_end = db.Column(db.String(16), nullable=False)
    port_ibo = db.Column(db.Integer, default=8300)
    overlay_type = db.Column(db.String(16), default='vxlan')
    nodes = db.relationship("Node", backref="zgc")
    vpcs = db.relationship("Vpc", backref="zgc")

    def to_json(self):
        return {
            'id': self.id,
            'zgc_id': self.zgc_id,
            'name': self.name,
            'description': self.description,
            'ip_start': self.ip_start,
            'ip_end': self.ip_end,
            'port_ibo': self.port_ibo,
            'overlay_type': self.overlay_type,
            'nodes': [],
            'vpcs': []
        }


class Node(db.Model):

    __tablename__ = 'nodes'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    zgc_id = db.Column(db.String(64), db.ForeignKey('zgcs.zgc_id'),
                       nullable=False)
    node_id = db.Column(db.String(64), unique=True, nullable=False)
    name = db.Column(db.String(128), unique=True, nullable=False)
    description = db.Column(db.String(255), nullable=True)
    ip_control = db.Column(db.String(16), nullable=False)
    id_control = db.Column(db.String(64), nullable=False)
    pwd_control = db.Column(db.String(64), nullable=False)
    inf_tenant = db.Column(db.String(16), nullable=False)
    mac_tenant = db.Column(db.String(18), nullable=False)
    inf_zgc = db.Column(db.String(16), nullable=False)
    mac_zgc = db.Column(db.String(18), nullable=False)

    def to_json(self):
        return {
            'id': self.id,
            'zgc_id': self.zgc_id,
            'node_id': self.node_id,
            'name': self.name,
            'description': self.description,
            'ip_control': self.ip_control,
            'inf_tenant': self.inf_tenant,
            'inf_zgc': self.inf_zgc
        }


class Vpc(db.Model):

    __tablename__ = 'vpcs'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    zgc_id = db.Column(db.String(64), db.ForeignKey('zgcs.zgc_id'),
                       nullable=False)
    vpc_id = db.Column(db.String(64), unique=True, nullable=False)
    vni = db.Column(db.Integer, nullable=False)
    ports = db.relationship("Port", backref="vpc", lazy=True)

    def to_json(self):
        return {
            'id': self.id,
            'zgc_id': self.zgc_id,
            'vpc_id': self.vpc_id,
            'vni': self.vni,
            'ports': []
        }


class Host(db.Model):

    __tablename__ = 'hosts'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    host_id = db.Column(db.String(64), unique=True, nullable=False)
    mac_node = db.Column(db.String(18), nullable=False)
    ip_node = db.Column(db.String(16), nullable=False)
    ports = db.relationship("Port", backref="host", lazy=True)

    def to_json(self):
        return {
            'id': self.id,
            'host_id': self.host_id,
            'mac_node': self.mac_node,
            'ip_node': self.ip_node,
            'ports': []
        }


class Port(db.Model):

    __tablename__ = 'ports'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    port_id = db.Column(db.String(64), unique=True, nullable=False)
    mac_port = db.Column(db.String(18), nullable=False)
    vpc_id = db.Column(db.String(64), db.ForeignKey(
        'vpcs.vpc_id'), nullable=False)
    host_id = db.Column(db.String(64), db.ForeignKey(
        'hosts.host_id'), nullable=False)
    eps = db.relationship("EP", backref="port", lazy="joined")

    def to_json(self):
        return {
            'id': self.id,
            'port_id': self.port_id,
            'mac_port': self.mac_port,
            'vpc_id': self.vpc_id,
            'host_id': self.host_id,
            'eps': []
        }


class EP(db.Model):

    __tablename__ = 'eps'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    ip = db.Column(db.String(16), nullable=False)
    vip = db.Column(db.String(16), nullable=False)
    port_id = db.Column(db.String(64), db.ForeignKey(
        'ports.port_id'), nullable=False)

    def to_json(self):
        return {
            'id': self.id,
            'ip': self.ip,
            'vip': self.vip,
            'port_id': self.port_id
        }
