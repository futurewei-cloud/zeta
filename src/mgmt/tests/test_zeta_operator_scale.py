import unittest
from tests.common.k8s_helper import K8sHelper
import logging
import os
import json
from time import sleep

logger = logging.getLogger()


class test_zeta_operator_scale(unittest.TestCase):

    def setUp(self):
        logger.info("")
        self.test_name = "test_operator_errors"
        self.api = K8sHelper()
        self.api.print_operator_errors()
        self.test_dft = "dft0"
        self.test_chain = self.api.get_single_custom_object("chain")
        self.sleep_delay = 2

    def tearDown(self):
        pass

    def test_zeta_operator_scale(self):
        self.assertEqual(0, self.api.check_errors())

        logger.info("Testing FWD scale up")
        self.api.scale_zeta_object(self.test_dft, "dfts", "numfwds", 6)
        self.assertEqual(0, self.api.check_errors())
        sleep(self.sleep_delay)
        self.assertEqual(
            6, self.api.count_custom_object("fwds", self.test_dft))

        logger.info("Testing FWD scale down")
        self.api.scale_zeta_object(self.test_dft, "dfts", "numfwds", 3)
        self.assertEqual(0, self.api.check_errors())
        sleep(self.sleep_delay)
        self.assertEqual(
            3, self.api.count_custom_object("fwds", self.test_dft))

        logger.info("Testing FTN scale up {}".format(self.test_chain))
        self.api.scale_zeta_object(self.test_chain, "chains", "size", 3)
        self.assertEqual(0, self.api.check_errors())
        sleep(self.sleep_delay)
        self.assertEqual(3, self.api.count_custom_object(
            "ftns", self.test_chain))

        logger.info("Testing FTN scale down {}".format(self.test_chain))
        self.api.scale_zeta_object(self.test_chain, "chains", "size", 1)
        self.assertEqual(0, self.api.check_errors())
        sleep(self.sleep_delay)
        self.assertEqual(1, self.api.count_custom_object(
            "ftns", self.test_chain))

        logger.info("Testing chain scale up")
        self.api.scale_zeta_object(self.test_dft, "dfts", "numchains", 6)
        self.assertEqual(0, self.api.check_errors())
        sleep(self.sleep_delay)
        self.assertEqual(6, self.api.count_custom_object(
            "chains", self.test_dft))

        logger.info("Testing chain scale down")
        self.api.scale_zeta_object(self.test_dft, "dfts", "numchains", 3)
        self.assertEqual(0, self.api.check_errors())
        sleep(self.sleep_delay)
        self.assertEqual(3, self.api.count_custom_object(
            "chains", self.test_dft))
