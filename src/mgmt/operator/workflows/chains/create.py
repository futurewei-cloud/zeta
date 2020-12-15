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


def chain_create(task, chain, name, body, spec):
    logger.info("Creating Chain {}!".format(name))
    if not chain:
        chain = chains_opr.get_stored_obj(name, spec)
    if not dfts_opr.store.contains_obj(chain.dft, KIND.dft):
        task.raise_temporary_error(
            "Parent DFT {} not yet created!".format(chain.dft))
    ftns_opr.create_n_ftns(chain)
    chain.tail = chain.ftns[chain.size - 1]
    chain.head = chain.ftns[0]

    # Add chain to parent DFT table, generate maglev, update kubernetes dft obj
    dft = dfts_opr.store.get_obj(chain.dft, KIND.dft)
    dft.maglev_table.add(chain.id)
    dft.table = dft.maglev_table.get_table()
    dft.update_obj()
    return chain
