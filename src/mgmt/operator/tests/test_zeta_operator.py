import unittest
from tests.common.k8s_helper import K8sHelper
import logging
import os

logger = logging.getLogger()
logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"))


class test_operator_errors(unittest.TestCase):

    def setUp(self):
        logger.info("")
        self.test_name = "test_operator_errors"
        self.api = K8sHelper()
        self.api.print_operator_errors()

    def tearDown(self):
        pass

    def test_operator_errors(self):
        self.assertEqual(0, int(self.api.count_operator_errors()))
