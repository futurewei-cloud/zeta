# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

import logging
from common.workflow import *
from operators.chains_operator import *
from operators.dfts_operator import *
from operators.ftns_operator import *

logger = logging.getLogger()
chains_opr = ChainOperator()
dfts_opr = DftOperator()
ftns_opr = FtnOperator()


def chain_delete(task, chain, name, body, spec):
    logger.info("Deleting Chain {}!".format(name))
    if not chain:
        chain = chains_opr.get_stored_obj(name, spec)

    # Delete chain from parent DFT table, generate maglev, update kubernetes dft obj
    dft = dfts_opr.store.get_obj(chain.dft, KIND.dft)
    dft.maglev_table.remove(chain.id)
    dft.table = dft.maglev_table.get_table()
    dft.update_obj()
    return chain
