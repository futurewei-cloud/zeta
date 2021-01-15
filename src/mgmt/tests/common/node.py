import docker
import os
import logging
from time import sleep
from src.mgmt.tests.common.common import logger, CONSTANTS


class Node:

    def __init__(self, name, network, node_type):
        self.name = name
        self.network = network
        self.ports = []
        self.container = None
        self.ip = None
        self.mac = None
        if node_type == CONSTANTS.ACA_NODE:
            self._start_aca_node()
        logger.info("Node {} Created. IP {} MAC {}".format(
            self.name, self.ip, self.mac))

    def __del__(self):
        self.run("killall5 -2")
        sleep(1)
        self.delete_container()

    def delete_container(self):
        if self.container:
            self.container.stop()
            self.container.remove()

    def _start_aca_node(self):
        """
        Create and initialize an ACA docker container.
        """
        docker_client = docker.from_env()
        docker_image = "aca_build0:latest"  # Image from building ACA

        # Mount current working directory to /mnt/Zeta
        mount_pnt = docker.types.Mount("/mnt/Zeta",
                                       os.getcwd(),
                                       type='bind')
        mount_modules = docker.types.Mount("/lib/modules",
                                           "/lib/modules",
                                           type='bind')

        # Create the container with necessary privileges
        container = docker_client.containers.create(
            docker_image,
            '/bin/bash',
            tty=True,
            stdin_open=True,
            auto_remove=False,
            mounts=[mount_pnt, mount_modules],
            privileged=True,
            cap_add=["SYS_PTRACE", "NET_ADMIN"],
            name=self.name,
            network=self.network,
            security_opt=["seccomp=unconfined"],
        )
        container.start()
        container.reload()

        # Setup OVS on the ACA build container
        container.exec_run("apt-get install -y openvswitch-switch")
        container.exec_run("/etc/init.d/openvswitch-switch restart")
        container.exec_run("ovs-vswitchd --pidfile --detach")

        # Run the aca in the background
        container.exec_run(
            "./mnt/Zeta/src/extern/alcor-control-agent/build/bin/AlcorControlAgent", detach=True)
        self.container = container
        self.ip = self.container.attrs['NetworkSettings']["Networks"][self.network]['IPAddress']
        self.mac = self.container.attrs['NetworkSettings']["Networks"][self.network]['MacAddress']

    def run(self, cmd, detach=False):
        """
        Runs a command on the container
        """

        ret_value = None
        out = self.container.exec_run(cmd, detach=detach)
        if not detach:
            ret_value = (out.exit_code, out.output.decode("utf-8"))

        logger.info("[Node {}]: running: {}".format(self.name, cmd))
        if (not detach and ret_value[0] == 1):
            logger.error("[Node {}]: {}".format(self.name, ret_value[1]))

        if not detach:
            logger.info(
                "[Node {}]: running\n    command:\n {}, \n    exit_code: {},\n    output:\n {}".format(self.name, cmd, ret_value[0], ret_value[1]))
        return ret_value
