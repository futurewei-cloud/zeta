from kubernetes import client, config
import subprocess
import yaml
import logging
import os

logger = logging.getLogger()
logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"))


class K8sHelper:

    def __init__(self):
        config.load_kube_config()
        self.k8sapi = client.CoreV1Api()
        self.custom_obj_api = client.CustomObjectsApi()
        self.operator_logs_cmd = "kubectl get pods | grep zeta-operator | awk '{print $1}' | xargs -i kubectl logs {}"
        self.manager_logs_cmd = "kubectl get pods | grep zeta-manager| awk '{print $1}' | xargs -i kubectl logs {}"
        self.operator_pod_name = self.run_cmd(
            "kubectl get pods | grep zeta-operator | awk '{print $1}'")
        self.manager_pod_name = self.run_cmd(
            "kubectl get pods | grep zeta-manager | awk '{print $1}'")

    def run_cmd(self, cmd_list):
        result = subprocess.Popen(cmd_list, shell=True, stdout=subprocess.PIPE)
        text = result.stdout.read().decode().rstrip()
        return text

    def patch_zeta_object(self, name, plural, body):
        try:
            response = self.custom_obj_api.patch_namespaced_custom_object(
                group="zeta.com",
                version="v1",
                namespace="default",
                plural=plural,
                name=name,
                body=body
            )
            return response
        except Exception as e:
            logger.info(
                "Unable to patch custom object {}. Exception: {}".format(name, e))

    def get_zeta_object(self, name, plural):
        try:
            response = self.custom_obj_api.get_namespaced_custom_object(
                group="zeta.com",
                version="v1",
                namespace="default",
                plural=plural,
                name=name
            )
            return response
        except Exception as e:
            logger.info(
                "Unable to get custom object {}. Exception: {}".format(name, e))

    def count_operator_permanent_errors(self):
        cmd = "{} | grep 'failed permanently' -c".format(
            self.operator_logs_cmd)
        return self.run_cmd(cmd)

    def print_operator_errors(self):
        logger.info("Printing errors if any!")
        cmd = "{} | grep -E '^[A-Z][a-z].*Error:'".format(
            self.operator_logs_cmd)
        logger.info(self.run_cmd(cmd))

    def count_operator_errors(self):
        cmd = "{} | grep -E '^[A-Z][a-z].*Error:' -c".format(
            self.operator_logs_cmd)
        return self.run_cmd(cmd)

    def check_errors(self):
        self.print_operator_errors()
        return int(self.count_operator_permanent_errors()) + int(self.count_operator_errors())

    def scale_zeta_object(self, name, plural, field, amount):
        body = {
            "spec": {
                field: amount
            }
        }
        self.patch_zeta_object(name, plural, body)

    def count_custom_object(self, object_type,  object_name):
        cmd = "kubectl get {} | grep -c {}".format(object_type, object_name)
        return int(self.run_cmd(cmd))

    def get_single_custom_object(self, object_type):
        cmd = "kubectl get {} | grep dft | awk 'NR==1{{print $1}}'".format(
            object_type)
        return self.run_cmd(cmd)
