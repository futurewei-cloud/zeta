# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
import json
from common.constants import *
from common.common import *

logger = logging.getLogger()


class KubeObject(object):

    def __init__(self, name, obj_api, opr_store, spec=None):
        self.name = name
        self.obj_api = obj_api
        self.store = opr_store
        self.status = OBJ_STATUS.obj_status_init
        self.id = ""
        self.plural = ""
        self.kind = ""

    def get_name(self):
        return self.name

    def get_plural(self):
        return self.plural

    def get_kind(self):
        return self.kind

    def create_obj(self):
        return kube_create_obj(self)

    def update_obj(self):
        return kube_update_obj(self)

    def delete_obj(self):
        return kube_delete_obj(self)

    def watch_obj(self, watch_callback):
        return kube_watch_obj(self, watch_callback)

    def set_status(self, status):
        self.status = status

    def store_update_obj(self):
        self.store.update_obj(self)
