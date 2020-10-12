# Basic Usage

To start running Zeta, first you will need a Kubernetes cluster. Running Zeta will  install experimental features on your cluster. **It is recommended that you use a test cluster to run Zeta.**

## Prerequisites

### Kind (Kubernetes in Docker)

The recommended way to try out Zeta is with Kind.
Kind can be used to run a multi-node Kubernetes cluster with Docker containers locally on your machine.
You can find [instructions for installing Kind on it's official site here.](https://kind.sigs.k8s.io/docs/user/quick-start/)

* Enter the zeta directory and run the ```bootstrap.sh``` script.
    * This script will install the neccessary components to compile Zeta, and run it's unit tests. These include
        * [Clang-7](https://clang.llvm.org) (For code compilation)
        * [Llvm-7](https://llvm.org) (For code compilation)
         * [Cmocka](https://cmocka.org) (For unit testing)
        * [Build Essentials](https://packages.ubuntu.com/xenial/build-essential) (For code compilation)
        * [Python](https://www.python.org) (For running the management plane and tests)
        * [Docker](https://www.docker.com) (For management plane and tests)
    * There is also an optional kernel update included in the script if you wish to update to a compatible version

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
$ ./kind-setup.sh dev
```

This script does the following:

* Create a multi-node local Kubernetes cluster with Kind.
* Build the Kind-Node, Zeta-Daemon, and Zeta-Operator docker images
* Apply all of the Zeta [CRDs](https://kubernetes.io/docs/concepts/extend-kubernetes/api-extension/custom-resources/).
* Deploy the Zeta [Operators](https://kubernetes.io/docs/concepts/extend-kubernetes/operator/).
* Deploy the Zeta Daemon

### Linux Kernel Update

For TCP to function properly, you will need to update your Kernel version to at least 5.6-rc2 on every node. A script, ```kernel_update.sh``` is provided in the Zeta repo to download and update your machine's kernel if you do not wish to build the kernel source code yourself.
