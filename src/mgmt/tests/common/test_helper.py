# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Phu Tran          <@phudtran>

from src.mgmt.tests.common.common import logger


def do_ping_test(test, node1, node1_port, node2, node2_port, both_ways=True):
    logger.info("Test {}: {} do ping test {}".format(
        type(test).__name__, "="*10, "="*10))
    logger.info("Test: {} can ping {}".format(node1_port, node2_port))
    exit_code = ping(node1, node1_port, node2_port)
    test.assertEqual(exit_code, 0)
    if both_ways:
        logger.info("Test: {} can ping {}".format(node2_port, node1_port))
        exit_code = ping(node2, node2_port, node1_port)
        test.assertEqual(exit_code, 0)


def ping(node, node_port, remote_port, count=1):
    return node.run("ping -I {} {} -c{}".format(node_port, remote_port, count))[0]
