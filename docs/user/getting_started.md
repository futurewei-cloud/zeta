# Basic Usage

Zeta system itself includes two self-contained clusters:
- One k8s micro-service cluster hosting control plane services including
  - Zeta control plane service such as zeta-operator and zeta-manager
  - Zeta dependency services such as ingress, postgres
  - responsible for providing Zeta NB API and managing Zeta data plane (ZGC)
- One Zeta Gateway Cluster (aka ZGC) for Zeta Data plane
  - Made of bare-metal servers or VMs in production environment
  - Can be Containers in "Kind" deployment scenario for local functional test on a single host
  - ZGC hosts Zeta Data Plane functionalities, controlled by Zeta control plane through its NB RPC interface

To integrate zeta system into existing cloud/DC infrastructure, please refer to our sibling project [Alcor](https://github.com/futurewei-cloud/alcor) for reference which integrates Zeta for networking services in OpenStack environment.

To demonstrate Zeta functionality End-to-end without the burden of complicated external dependencies such as OpenStack, we created a fully container based single host deployment scenario, which is also used for development and sanity test, detailed in [here](https://github.com/futurewei-cloud/zeta/blob/main/docs/user/getting_started.md#deploy-to-local-kind-kubernetes-in-docker-cluster).

To start developing/deploying Zeta, you will need:
- a host as **deployer**:
  - to build and package Zeta system
  - to deploy zeta system to target site
  - to manage or interact with deployed Zeta system
  - For reference we are using Ubuntu Bionic VM
- a target site to deploy Zeta into as **site**:
  - The site has dedicated bare-metal servers/VMs for zeta control plane cluster and data plane zeta gateway cluster (ZGC)
  - the target site can be implemented fully containerized and co-hosted with **deployer** as in the [**Kind** deployment scenario](https://github.com/futurewei-cloud/zeta/blob/main/docs/user/getting_started.
  - Zeta **deployer** script will deploy and provision the two Zeta clusters on the target Zeta servers/VMs

## Prerequisites
**Deployer** host Linux environment:
- Ubuntu: tested with 18.04 LTS
- Python3 & Pip3
- git: tested with version 2.25.1
- [Docker](https://www.docker.com): tested with version 19.03.12, don't use snap installation. Also
  make sure your login user ID is added in docker group so you can run docker without sudo:
  ```
  sudo groupadd docker
  sudo usermod -aG docker $USER
  newgrp docker
  ```
- [Kind](https://kind.sigs.k8s.io): tested with version 0.8.1
  ```
  curl -Lo ./kind https://kind.sigs.k8s.io/dl/v0.9.0/kind-linux-amd64
  chmod +x ./kind
  sudo mv ./kind /usr/local/bin/kind
  ```
- [Kubectl](https://kubectl.docs.kubernetes.io): tested with version 1.18
  ```
  curl -LO https://storage.googleapis.com/kubernetes-release/release/v1.19.0/bin/linux/amd64/kubectl
  chmod +x ./kubectl
  sudo mv ./kubectl /usr/local/bin/kubectl
  ```
- Python modules for remote deployment:
  ```
  pip3 install boto boto3 botocore ansible
  ```

Target **site** host Linux environment:
- Ubuntu server: tested with 18.04 LTS
- The dedicated Zeta servers/VMs are physically networked according to [Zeta design document](https://github.com/futurewei-cloud/zeta/blob/main/docs/design/zeta_system_design.md#441-best-practices)
md#deploy-to-local-kind-kubernetes-in-docker-cluster) detailed below.
- Kernel for ZGC nodes will be upgraded to minimal 5.6.0-rc2 by **deployer** during deployment procedure.

## Build and Unit Testing
1. Clone the Zeta repository:
    ```
    git clone --recurse-submodules git@github.com:futurewei-cloud/zeta.git
    cd zeta
    ```
2. Build and Run built-in Unit tests, all test cases should pass
    ```
    ./build.sh debug
    ```

## Deploy and Testing

### Deploy to Local Kind (Kubernetes in Docker) Cluster

The recommended way to try out Zeta is with Kind.
Kind can be used to run a multi-node Kubernetes cluster with Docker containers locally on your machine.
You can find [instructions for installing Kind on its official site here.](https://kind.sigs.k8s.io/docs/user/quick-start/)

### Kind Cluster Environment
##### Deployment
**Note**: Before proceeding with the setup script below, please make sure you have [kind](https://kind.sigs.k8s.io/docs/user/quick-start/) and [kubectl](https://kubernetes.io/docs/tasks/tools/install-kubectl/) installed.
Validate your kind and kubectl installations by running:

```
$ kind --version

$ kubectl version --client
```

If you are testing out Zeta with Kind, a script has been included to setup a local Kubernetes cluster, and install all of the components needed to run Zeta. Simply run the script below in the Zeta directory.

```
$ ./deploy/full_deploy.sh -d kind
```

This script does the following:

* Create a multi-node (default is 3) local Kubernetes cluster with Kind. By default following 3 pods are created:
  - postgres
  - zeta-manager
  - zeta-operator
* Build the Kind-Node (zeta_node), and Zeta-Operator (zeta_droplet) docker images
* Apply all of the Zeta [CRDs](https://kubernetes.io/docs/concepts/extend-kubernetes/api-extension/custom-resources/).
* Deploy the Zeta [Operators](https://kubernetes.io/docs/concepts/extend-kubernetes/operator/).
* You can customize kind cluster by modifying deploy/playbooks/inventories/vars/site_kind.yml
* Note: First time you deploy, the tool will ask you to setup deploy secrets, follow instruction and press "enter" if you don't have the info asked.

Once deployed, local kubeconfig is automatically updated for the new cluster, so you can use kubectl to manage the cluster.
If you made some changes to Zeta, you can re-deploy zeta services cluster created above using following script. This will be much faster.
```
$ ./deploy/zeta_deploy.sh -d kind
```
To remove the cluster and all resources:
```
$ ./deploy/full_deploy.sh -r kind
```

#### Testing with Alcor-Control-Agent
More information about the [Alcor-Control-Agent](https://github.com/futurewei-cloud/alcor-control-agent).
Once Zeta has been fully deployed and ready, we can run a ping test with the help of the ACA (Alcor-Control-Agent).
Before proceeding with the ACA test, first we will need to build its binaries.
From the Zeta directory run the following
```
$ ./src/extern/alcor-control-agent/build/build.sh
```
Once ACA is done building successfully, the docker image aca_build0 is created. From the Zeta directory, you can now run a simple ping test with the command below.
```
python3 -W ignore -m unittest  src/mgmt/tests/test_zeta_aca_ping.py

Above test creates two containers (aca_node_1 & aca_node_2) from the aca_build0 image. These two container nodes will ping each other handful of times. A successful run of the test shows the time in seconds it took to complete the test.
```
### Remote public Cloud deployment

#### Deployment to AWS EC2 as example

**Note**: Before proceeding with the setup script below, please make sure you have [kubectl](https://kubernetes.io/docs/tasks/tools/install-kubectl/) installed and you have AWS credentials ready.

1. Setup secret vault to keep AWS access credentials stored locally encrypted:

```
$ ./deploy/env_setup.sh
```

2. Make necessary changes for the AWS site to be created for you in deploy/playbooks/inventories/vars/site_aws.yml
3. Setup AWS site clusters and deploy Zeta

```
$ ./deploy/full_deploy.sh -d aws
```
4. Once deployed, local kubeconfig is automatically updated for the new cluster, so you can use kubectl to manage the remote zeta control plane services
5. To remove Zeta deployment:
```
$ ./deploy/full_deploy.sh -r aws
```
6. If you made some changes to Zeta, you can re-deploy only the zeta services to the target site created in step 3, this will be much faster
```
$ ./deploy/zeta_deploy.sh -d aws
```

### Remote Bare-metal deployment

#### Deployment to Lab server pool as example

**Note**: Before proceeding with this scenario, please review and change site inventory templates according to your setup in follow files:
- deploy/playbooks/inventories/vars/site_lab.yml
- deploy/playbooks/inventories/hosts_lab

Simply run the script below in the Zeta directory to deploy Zeta to the remote bare-metal site.

1. Setup secret vault to keep LAB access credentials stored locally encrypted:

```
$ ./deploy/env_setup.sh
```

2. Make necessary changes for the LAB site to be created for you in deploy/playbooks/inventories/vars/site_lab.yml and deploy/playbooks/inventories/hosts_lab
3. Setup LAB site clusters and deploy Zeta

```
$ ./deploy/full_deploy.sh -d lab
```
4. Once deployed, local kubeconfig is automatically updated for the new cluster, so you can use kubectl to manage the remote zeta control plane services
5. To remove Zeta deployment:
```
$ ./deploy/full_deploy.sh -r lab
```
6. If you made some changes to Zeta, you can re-deploy only the zeta services to the target site created in step 3, this will be much faster
```
$ ./deploy/zeta_deploy.sh -d lab
```
