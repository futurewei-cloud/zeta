# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from common.workflow import *
from operators.chains_operator import *
from operators.droplets_operator import *
from operators.dfts_operator import *
from operators.ftns_operator import *
from operators.fwds_operator import *

dfts_opr = DftOperator()
chains_opr = ChainOperator()
ftns_opr = FtnOperator()
fwds_opr = FwdOperator()
droplets_opr = DropletOperator()


def fwd_delete(task, fwd, name, body, spec, diff):
    logger.info("Deleting Fwd {}!".format(name))
    if not fwd:
        fwd = fwds_opr.get_stored_obj(name, spec)
    droplet_obj = droplets_opr.store.get_obj(fwd.droplet, KIND.droplet)
    # Don't need to run delete_dft and delete chain, unload of XDP cleans it up
    droplet_obj.rpc.unload_transit_xdp()
    droplets_opr.unassign_droplet(fwd)
    return fwd
