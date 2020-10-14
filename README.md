# Zeta [![Build Status](https://travis-ci.org/futurewei-cloud/zeta.svg?branch=main)](https://travis-ci.org/futurewei-cloud/zeta)

Zeta is a distributed platform for developing and deploying complex, elastic, and highly available multi-tenant network services. By migrating existing hardware gateways functionality to Zeta, a cloud provider can decouple
network services expansion, maintenance, configuration, and failure domain from the data-center network. Zeta’s programming and deployment models allow rapid upgrade of network services and the introduction of new services
with an optimized cost model. Although being a shared platform for multiple network services, both stateless and state-full services can scale independently.

# Getting Started
Please see our getting started guide [here](docs/user/getting_started.md).

# Introduction
Hardware-based middleboxes are expensive, inextensible, and difficult to operate and scale in a cloud environment.
a naive approach of deploying existing software-based middleboxes in virtual machines or containers to replace
hardware gateways is not sufficient and results in severe problems in production systems. Existing software-based
middleboxes or virtual switches (e.g., ClickOS, OVS), maintain a local shared connection state. This model is
too constraining to service availability and scaling. Without maintaining a consistent connection state across several
middleboxes, adding or removing a middlebox – during failure recovering or scale-out/in – results in connection loss,
and worse in inconsistent packet forwarding decisions.

We propose to separate connection tracking from middlebox decisions so that we have two packet processing
layers that scale and fail independently from each other. The connection tracking layer is an in-network key-value
store that implements a Distributed Flow Table (DFT). Unlike a typical key-value store, the DFT embeds its transactions
in the regular packet flow without the need to establish connections to the DFT nodes, buffer packets, or
develop an external protocol to support the DFT operations. The DFT maintains consistent connection states across
replicas and slowly scales-out according to the rate of new connections or connection closing. The second is a
stateless forwarding layer that executes packet forwarding decisions for the already established connections and
bounces packets for transmission. Individual nodes within the forwarding layer fail independently and scale-out
rapidly according to the total packet rate of a network service. The two layers together form an extensible, elastic,
and fault-tolerant distributed system of middleboxes that replaces existing hardware gateways, which we name
Zeta.

At high level, we hope Zeta will be able to accomplish the following:
1. Modular middleboxes with XDP programs that are chained to form various types of network service decisions
2. Local shared configuration across multiple XDP programs that reduces the need for a packet to traverse multiplenodes to make a decision on a new flow
3. Overlay protocol integrated within the data-plane to provide consistent connection state
4. Built-in multi-tenancy that allows infrastructure sharing of network services and middleboxes without the
need to isolate middleboxes in virtual machines or containers, hence improves cost
5. Native data-plane failure detection that minimizes the time to recover a connection by means of gossip protocols
