import docker
import os
import logging
from time import sleep

logger = logging.getLogger()


class Node:

    def __init__(self, name, network):
        self.name = name
        self.network = network
        self.ports = []
        self.container = None
        self.ip = None
        self.mac = None
        self._start()
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

    def _start(self):
        """
        Create and initialize a docker container.
        """
        cwd = os.getcwd()

        # get a docker client
        docker_client = docker.from_env()
        docker_image = "fwnetworking/zeta_aca:latest"
        mount_pnt = docker.types.Mount("/mnt/Zeta",
                                       cwd,
                                       type='bind')
        mount_modules = docker.types.Mount("/lib/modules",
                                           "/lib/modules",
                                           type='bind')

        # Create the container with necessary privileges
        # Mount current working directory to /mnt/Zeta
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

        # # Restart needed services
        container.exec_run("/etc/init.d/openvswitch-switch restart")

        # # Run the aca in the background
        # container.exec_run("./AlcorControlAgent",
        #                    detach=True)
        self.container = container
        self.ip = self.container.attrs['NetworkSettings']["Networks"][self.network]['IPAddress']
        self.mac = self.container.attrs['NetworkSettings']["Networks"][self.network]['MacAddress']

    def run(self, cmd, detach=False):
        """
        Runs a command directly on the container
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
