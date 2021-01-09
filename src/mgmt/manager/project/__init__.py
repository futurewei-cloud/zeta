# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: Factory pattern for Zeta-manager NBI API app
#


import os
import logging

from flask import Flask, jsonify, url_for, request, Response
from flask_sqlalchemy import SQLAlchemy
from flask_migrate import Migrate
from flask_cors import CORS
from .api import settings
import logging

# instantiate the extensions
db = SQLAlchemy()
migrate = Migrate()


def create_app(script_info=None):

    logging.basicConfig(
        level=logging.DEBUG,
        format='%(asctime)s-%(levelname)s:%(name)s:%(funcName)s:%(lineno)d — %(message)s')

    # instantiate the app
    app = Flask(__name__)
    # enable CORS
    CORS(app)

    # set config
    app_settings = os.getenv('APP_SETTINGS')
    app.config.from_object(app_settings)
    print(app.config["STAGE"])

    # set up extensions
    db.init_app(app)
    migrate.init_app(app, db)

    settings.init()

    # register blueprints
    from project.api.books import books_blueprint
    app.register_blueprint(books_blueprint)

    from project.api.zgcs import zgcs_blueprint
    app.register_blueprint(zgcs_blueprint)

    from project.api.nodes import nodes_blueprint
    app.register_blueprint(nodes_blueprint)

    from project.api.vpcs import vpcs_blueprint
    app.register_blueprint(vpcs_blueprint)

    from project.api.ports import ports_blueprint
    app.register_blueprint(ports_blueprint)

    from project.api.hosts import hosts_blueprint
    app.register_blueprint(hosts_blueprint)

    from project.api.eps import eps_blueprint
    app.register_blueprint(eps_blueprint)

    # shell context for flask cli
    @app.shell_context_processor
    def ctx():
        return {'app': app, 'db': db}

    return app
