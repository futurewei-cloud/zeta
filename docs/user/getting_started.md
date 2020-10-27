# Basic Usage

To start running Zeta, first you will need a Kubernetes cluster. Running Zeta will  install experimental features on your cluster. **It is recommended that you use a test cluster to run Zeta.**

## Prerequisites
Host Linux environment:
- Ubuntu: tested with 18.04 LTS with kernel upgrade to 5.6.0-rc2, see Kernel Upgrade note below
- git: tested with version 2.25.1
- [Docker](https://www.docker.com): tested with version 19.03.12
- [Kind](https://kind.sigs.k8s.io): tested with version 0.8.1
- [Kubectl](https://kubectl.docs.kubernetes.io): tested with version 1.18

Also please make sure your login user ID is added in docker group so you can run docker without sudo:
```
sudo groupadd docker
sudo usermod -aG docker $USER
newgrp docker 
```


### Kubernetes Cluster

For a pre-existing Kubernetes cluster, simply run provided script ```./deploy/deploy-mgmt.sh```.
This script will apply all of the CRDs and deploy the operator to the cluster.

### Networking Cluster

TBD

## Build and Unit Testing
1. Clone the Zeta repository:
    ```
    git clone git@github.com:futurewei-cloud/zeta.git
    ```
2. Enter the local repo:
    ```
    cd zeta
    ```
3. Build and Run built-in Unit tests
    ```
    ./build.sh debug
    ```

## Deploy and Testing

### Deploy to Local Kind (Kubernetes in Docker) Cluster

The recommended way to try out Zeta is with Kind.
Kind can be used to run a multi-node Kubernetes cluster with Docker containers locally on your machine.
You can find [instructions for installing Kind on it's official site here.](https://kind.sigs.k8s.io/docs/user/quick-start/)

#### New Kind Cluster
**Note**: Before proceeding with the setup script below, please make sure you have [kind](https://kind.sigs.k8s.io/docs/user/quick-start/) and [kubectl](https://kubernetes.io/docs/tasks/tools/install-kubectl/) installed.
Validate your kind and kubectl installations by running:

```
$ kind --version

$ kubectl version --client
```

If you are testing out Zeta with Kind, a script has been included to setup a local Kubernetes cluster, and install all of the components needed to run Zeta.
Simply run the script below in the Zeta directory.

```
$ ./deploy/deploy-to-kind.sh
```

This script does the following:

* Create a multi-node (default is 3) local Kubernetes cluster with Kind.
* Build the Kind-Node, and Zeta-Operator docker images
* Apply all of the Zeta [CRDs](https://kubernetes.io/docs/concepts/extend-kubernetes/api-extension/custom-resources/).
* Deploy the Zeta [Operators](https://kubernetes.io/docs/concepts/extend-kubernetes/operator/).

### Deploy to Remote K8S Cluster

**Note**: Before proceeding with the setup script below, please make sure you have [kubectl](https://kubernetes.io/docs/tasks/tools/install-kubectl/) installed and configured to access the remote K8S cluster.

Validate your kubectl installations by running:
```
$ kubectl version --client
```

Simply run the script below in the Zeta directory to deploy Zeta to the K8S cluster.

```
$ ./deploy/deploy-to-k8s.sh
```

## Linux Kernel Update

For TCP to function properly, you will need to update your Kernel version to at least 5.6-rc2 on every node. A script, ```kernelupdate.sh``` is provided in the Zeta repo to download and update your machine's kernel if you do not wish to build the kernel source code yourself.
```
./deploy/kernelupdate.sh
```
