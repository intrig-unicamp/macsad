********************
********************
OLD Readme:
----------
MACSAD
==========

The Multi-Architecture Compiler System for Abstract Dataplanes (MACSAD) is a P4 compiler that uses ODP aiming to archive portability of dataplane applications without compromising the target performance. MACSAD integrates the ODP APIs with P4, defining a programmable dataplane across multiple targets in an unified compiler system. MACSAD has a designed compiler module that generates an Intermediate Representation (IR) for P4 applications.

Follow the steps below to setup and run MACSAD on an Ubuntu 16.04.2 LTS and later.

Note: In this tutorial we are going to install the MACSAD at `/root` folder. 

To install MACSAD and it dependencies you can run our script (and skip to Part 3):

- `cd /root/`
- `./mac/install.sh`

Or you can follow the next steps. We strongly suggest to run our `install_pkgs.sh`.

# Part 1

---

## ODP installation:

MACSAD uses ODP for forwarding plane developement. Fist of all, we need to create a directory in the same folder where the MACSAD will be cloned:

- `mkdir tools`
- `mkdir tools/odp`

1. Download ODP v1.16.0 and compile it:

- `sudo apt-get install -y build-essential autoconf automake pkg-config libssl-dev libtool`
- `git clone -b v1.16.0.0 https://github.com/Linaro/odp`
- `cd odp`
- `./bootstrap`
- `./configure --disable-abi-compat --prefix=/root/tools/odp`
- `make`
- `make install`
- `ln -s /root/odp/helper /root/tools/odp`        
- `export ODP_SDK=/root/tools/odp`
- `cd ..`

# Part 2
---

## MACSAD installation

1. To run MACSAD we need P4-hlir submodule. Thus, At this step, we will clone MACSAD project and update/install the submodule. 

- `sudo apt-get install -y libpcap-dev python-scapy python-yaml graphviz python-setuptools`
- `git clone --recursive https://github.com/intrig-unicamp/mac.git`
- `cd mac`
- `cd p4-hlir`
- `python setup.py install`
- `cd ..`

For any issues with p4-hlir, please refer to its `README.md` file.

2. The p4 program needs to be translated for the MACSAD switch project. You can do this as below:

- `python src/transpiler.py examples/p4_src/l2_fwd.p4`

NOTE: This needs to be done everytime the P4 source file is modified or if any of the sugered file inside `src/hardware_indep` is changed.

3. Set environment variables and compile MACSAD:

- `export LD_LIBRARY_PATH=$ODK_SDK:$LD_LIBRARY_PATH`
- `make`

4. Set hugepages number and create interfaces for the test:

- `sudo sysctl -w vm.nr_hugepages=512`
- `./scripts/veth_create.sh`

# Part 3
---

## Running a simple test

We will use veth'x' interfaces with this example. Veth1 and veth2 will be part of the switch. We will send packet to veth0 and monitor at veth3. Similary we will send packet to veth3 and recieve the packets at veth0.

It is necessary to use the MAC addresses of veth1 and veth2 interfaces while forming the packets. 

Let us assume that the MAC addresses are: 

veth3 - a2:5e:37:ac:a1:7f

veth0 - fa:4f:e8:df:b1:5f

We need four terminals to perform this test.

### TERMINAL 1:

Build and start the minimalistic controller (required for installing flows in the switch):

- `cd src/hardware_dep/shared/ctrl_plane`
- `make mac_controller`
- `./mac_controller`

The controller will run in the foreground, so feel free to open another screen or terminal session.

### TERMINAL 2:

Start the MACSAD switch with veth interfaces 1 and 2

- `sudo ./macsad -i veth1,veth2 -c 0 -m 0 --out_mode 0`

### TERMINAL 3:

Now will use the veth interfaces created in step #5. `veth1` and `veth2` will be part of the switch. 
We will send packet to `veth0` (which is `veth1`'s pair) and monitor at `veth3` (which is `veth2`'s pair) for packets. Similary we will send packet to `veth3` and expect packets to arrive at `veth0`:

- `sudo python run_test.py`

You should see output similar to this:

- `$ sudo python run_test.py`
- `WARNING: No route found for IPv6 destination :: (no default route?)`
- `Sending 1 packets to  veth3 ...`
- `DISTRIBUTION:`
- `port veth0:   1 [ 100.0% ]`
- `port veth3:   1 [ 100.0% ]`
- `sendp(pkt1,iface="veth3",count=1);`

You should be able to catch the packet at veth1 using tcpdump/tshark in terminal 4. You can also verify the RX count using ifconfig.

NOTE: The packet processing logs can be seen at TERMINAL 2 as debug output of the switch.

Now send a packet from veth0 to veth3 and verify similarly at terminal 4.

- `pkt2 = Ether(dst='a2:5e:37:ac:a1:7f',src='fa:4f:e8:df:b1:5f')/IP(dst='192.168.0.2',src='192.168.0.1')`
- `sendp(pkt2,iface="veth0",count=1);`

The first packet with an unknown destination mac address will be broadcasted by the switch while the source mac address is learned. Now after the two packets were sent, the switch has already learned the mac addresses of veth0 and veth3. Now if we send those packets again, switch will forward those packets via corresponding ports instead of broadcasting them.

Notes:

- Update pip, setup tools to latest version  

    "pip install -U pip setuptools"

- Pip error:= "locale.Error: unsupported locale setting"  

   solution :=      "export LC_ALL=C"
