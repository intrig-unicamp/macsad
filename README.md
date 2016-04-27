MACSAD
==========
Follow the steps below to setup and run MACSAD on a Ubuntu 14.0.

# Part 1
---
##ODP:

MACSAD uses ODP for forwarding plane developement. Clone the ODP git project in any directory of your choice and compile.

- `git clone https://git.linaro.org/lng/odp.git`
- `cd odp`
- `git checkout tags/v1.9.0.0`
- `./bootstrap`
- `./configure`
- `make`

After compiling ODP, it is necessary to set the environment variable `ODP_SDK` as below:

- `export ODP_SDK=\<path_of_ODP

NOTE: It can also be added to the `~/.bashrc` file.

# Part 2
---

Clone the MACSAD project.

- `git clone https://github.com/intrig-unicamp/mac.git`
- `cd mac`
- `git checkout v0.1`

MACSAD has added P4-hlir as a submodule. Update the submodules as below:

- `git submodule init`
- `git submodule update`

##P4-Hlir:

Install p4-Hlir as below:

_Dependencies:_

- `sudo apt-get install python-yaml`
- `sudo apt-get install graphviz`
- `sudo python setup.py install`

NOTE: For any issues refer the README file under p4-hlir directory.

##Initial configurations necessary for MACSAD

1) The p4 program needs to be translated for the MACSAD switch project. You can do this as below:

- `cd mac`
- `python src/compiler.py examples/p4_src/l2_switch_test.p4`

NOTE: This needs to be done everytime the P4 source file is modified or if any of the sugered file inside `src/hardware_indep` is changed.

2) Create veth interfaces:

- `cd /script`
- `./host_create_veth_interfaces.sh`

3) Set the environment variable `LD_LIBRARY_PATH`:

- `export LD_LIBRARY_PATH=$ODP_SDK/lib/.libs:$LD_LIBRARY_PATH`

NOTE: It can also be added to the `~/.bashrc` file.

##Compile MACSAD
After this MACSAD can be compiled as below:

- `make`

# Part 3
---

## Running a simple test

We will use veth1.x and veth2.x pairs in this example. Veth1.1 and veth2.0 will be part of the switch. We will send packet to veth1.0 and monitor at veth2.1. Similary we will send packet to veth2.1 and recieve the packets at veth1.0.

Is necessary to use the MAC addresses of veth1.1 and veth2.0 interfaces while forming the packets. 
Let us assume that the MAC addresses are: 

veth2.0 - a2:5e:37:ac:a1:7f

veth1.1 - fa:4f:e8:df:b1:5f

We need four terminals to perform this test.

###TERMINAL 1:

Start the minimalistic Controller:

- `cd <mac>/src/hardware_dep/shared/ctrl_plane`
- `make clean`
- `make mac_controller`
- `./mac_controller`

###TERMINAL 2:

Start the MACSAD switch with veth interfaces 1.0 and 2.1

- `./mac_ad -i veth1.0,veth2.1`

NOTE: Run with root privilege.

###TERMINAL 3:
We use scapy to send packet to the switch interface. You can install the scapy package as below:

- `sudo apt-get install scapy`

First send a packet via veth2.0:

NOTE: Run scapy with root privilege.

- `sudo scapy`

Create a packet with veth2.0 as source and veth1.1 as destination as below:

- `pkt = Ether(dst='fa:4f:e8:df:b1:5f',src='a2:5e:37:ac:a1:7f')/IP(dst='192.168.0.1',src='192.168.0.2')`

Send one copy of the packet:

- `sendp(pkt,iface="veth2.0",count=1);`

You should be able to catch the packet at veth1.1 using tcpdump/tshark in terminal 4. You can also verify the RX count using ifconfig.

NOTE: The packet processing logs can be seen at TERMINAL 2 as debug output of the switch.

Now send a packet from veth1.1 to veth2.0 and verify similarly at terminal 4.

- `pkt = Ether(dst='a2:5e:37:ac:a1:7f',src='fa:4f:e8:df:b1:5f')/IP(dst='192.168.0.1',src='192.168.0.2')`

- `sendp(pkt,iface="veth1.1",count=1);`

The first packet with an unknown destination mac address will be broadcasted by the swith while the source mac address wis learned. Now after the two pakcets sent, the switch has already learned the mac addresses of veth1.1 and veth2.0. Now if we send those packets again, switch will forward those pakcets via corresponding ports instead of broadcasting them.
