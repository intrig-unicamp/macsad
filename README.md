MACSAD
==========
Follow the steps below to setup and run MACSAD on a Ubuntu 14.0.

# Part 1
---
##ODP:

MACSAD uses ODP for forwarding plane developement. Clone the ODP git project in any directory of your choice and compile.

- `git clone https://git.linaro.org/lng/odp.git`
- `cd odp`
- `git checkout tags/v1.10.1.0`
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
- `git checkout v0.2`

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
- `python src/transpiler.py examples/p4_src/l2_fwd.p4`

NOTE: This needs to be done everytime the P4 source file is modified or if any of the sugered file inside `src/hardware_indep` is changed.

2) Create veth interfaces:

- `cd /script`
- `./veth_create.sh`

3) Set the environment variable `LD_LIBRARY_PATH`:

- `export LD_LIBRARY_PATH=$ODP_SDK/lib/.libs:$LD_LIBRARY_PATH`

NOTE: It can also be added to the `~/.bashrc` file.

##Compile MACSAD
After this MACSAD can be compiled as below:

- `make`

# Part 3
---

## Running a simple test

We will use veth'x' interfaces with this example. Veth1 and veth2 will be part of the switch. We will send packet to veth0 and monitor at veth3. Similary we will send packet to veth3 and recieve the packets at veth0.

Is necessary to use the MAC addresses of veth1 and veth2 interfaces while forming the packets. 
Let us assume that the MAC addresses are: 

veth3 - a2:5e:37:ac:a1:7f

veth0 - fa:4f:e8:df:b1:5f

We need four terminals to perform this test.

###TERMINAL 1:

Start the minimalistic Controller:

- `cd <mac>/src/hardware_dep/shared/ctrl_plane`
- `make clean`
- `make mac_controller`
- `./mac_controller`

###TERMINAL 2:

Start the MACSAD switch with veth interfaces 1 and 2

- `./macsad -i veth1,veth2 -c 0 -m 0 --out_mode 0`

NOTE: Run with root privilege.

###TERMINAL 3:
We use scapy to send packet to the switch interface. You can install the scapy package as below:

- `sudo apt-get install scapy`

First send a packet via veth2:

NOTE: Run scapy with root privilege.

- `sudo scapy`

Create a packet with veth3 as source and veth0 as destination as below:

- `pkt1 = Ether(dst='fa:4f:e8:df:b1:5f',src='a2:5e:37:ac:a1:7f')/IP(dst='192.168.0.1',src='192.168.0.2')`

Send one copy of the packet:

- `sendp(pkt1,iface="veth3",count=1);`

You should be able to catch the packet at veth1 using tcpdump/tshark in terminal 4. You can also verify the RX count using ifconfig.

NOTE: The packet processing logs can be seen at TERMINAL 2 as debug output of the switch.

Now send a packet from veth0 to veth3 and verify similarly at terminal 4.

- `pkt2 = Ether(dst='a2:5e:37:ac:a1:7f',src='fa:4f:e8:df:b1:5f')/IP(dst='192.168.0.2',src='192.168.0.1')`

- `sendp(pkt2,iface="veth0",count=1);`

The first packet with an unknown destination mac address will be broadcasted by the swith while the source mac address wis learned. Now after the two pakcets sent, the switch has already learned the mac addresses of veth0 and veth3. Now if we send those packets again, switch will forward those pakcets via corresponding ports instead of broadcasting them.
