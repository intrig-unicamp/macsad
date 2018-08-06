## MACSAD [![License: Apache 2.0](https://img.shields.io/hexpm/l/plug.svg?style=for-the-badge)](LICENSE)

The Multi-Architecture Compiler System for Abstract Dataplanes (MACSAD) is a P4 compiler that uses ODP aiming to archive portability of dataplane applications without compromising the target performance. MACSAD integrates the ODP APIs with P4, defining a programmable dataplane across multiple targets in an unified compiler system. MACSAD has a designed compiler module that generates an Intermediate Representation (IR) for P4 applications.

To run MACSAD, follow the steps below. We have tested on Ubuntu 16.04.

Note: We suggest to install MACSAD using install.sh (inside scripts folder). This script cover all the necessary installation, running it allow the user to skip to section "Running MACSAD".

## ODP installation

MACSAD uses ODP for forwarding plane developement.

1. Download ODP v1.19.0.2, compile and install it:

        apt-get install -y build-essential autoconf automake pkg-config libssl-dev libconfig-dev libtool
        wget https://github.com/Linaro/odp/archive/v1.19.0.2.tar.gz
        tar xzvf v1.19.0.2.tar.gz; rm v1.19.0.2.tar.gz
        mv odp-1.19.0.2/ odp/
        cd odp
        ./bootstrap
        ./configure --disable-abi-compat --prefix=`pwd`/build
        make
        make install

2. Set the library and pkg-config info for ODP (make sure you are at the root
of the ODP build directory before running any of these commands):

        echo `pwd`/build/lib | sudo tee /etc/ld.so.conf.d/odp.conf
        ldconfig
        export PKG_CONFIG_PATH=`pwd`/build/lib/pkgconfig
        echo "export PKG_CONFIG_PATH=`pwd`/build/lib/pkgconfig" >> ~/.bashrc
        cd ..


## MACSAD installation

1. Clone the MACSAD project:

        apt install -y git
        git clone --recursive https://github.com/intrig-unicamp/macsad.git
        cd macsad
	
2. MACSAD has added `P4-hlir` as a submodule. Download/update and then install
it (along with its dependencies):

        apt-get install -y python-yaml graphviz python-setuptools
        cd p4-hlir
        python setup.py install --user

    For any issues with p4-hlir, please refer to its `README.md` file.

3. Translate the P4 program to MACSAD:
        
        cd ..
        python src/transpiler.py examples/p4_src/l2_fwd.p4

4. Install build dependencies, project dependencies and compile MACSAD:

        apt-get install -y autoconf build-essential pkg-config autoconf-archive libpcap-dev python-scapy
        ./autogen.sh
        ./configure
        make

5. Set hugepages number and create interfaces for the test:

        sysctl -w vm.nr_hugepages=512
        ./scripts/veth_create.sh

## Running MACSAD

1. Start the minimalistic controller (required for installing flows in the
switch):

        ./ctrl/mac_controller

    The controller will run in the foreground, so feel free to open another
    screen or terminal session.


2. Run the MACSAD switch:

        ./macsad -i veth1,veth2 -c 0 -m 0 --out_mode 0

    NOTE: To allow portability ODP sets a table size limit of 8192, to allow a higher limit:

        sed -i 's/max_queue_size = 8192/max_queue_size = YYY/' /root/odp/config/odp-linux-generic.conf

    Where YYY is the new limit. Then, odp needs to be recompiled. However, this will impact the memory usage and it has a limit of up to 1048576.

3. Now open a new terminal and we will use the veth interfaces created in step #5. `veth1` and `veth2` will
be part of the switch. We will send packet to `veth0` (which is `veth1`'s pair)
and monitor at `veth3` (which is `veth2`'s pair) for packets. Similary we will
send packet to `veth3` and expect packets to arrive at `veth0`:

        cd /root/macsad
        python run_test.py

    You should see output similar to this:

        $ python run_test.py
        WARNING: No route found for IPv6 destination :: (no default route?)
        Sending 1 packets to  veth3 ...
        DISTRIBUTION:
        port veth0:   1 [ 100.0% ]
        port veth3:   1 [ 100.0% ]

Optionally we can manually build our packet using scapy, this step is explained on our next section.

## MANUALLY BUILD/SEND PACKETS USING SCAPY (OPTIONAL)

We will use veth'x' interfaces with this example. Veth1 and veth2 will be part of the switch. We will send packet to veth0 and monitor at veth3. Similary we will send packet to veth3 and recieve the packets at veth0.

It is necessary to use the MAC addresses of veth1 and veth2 interfaces while forming the packets. 

Let us assume that the MAC addresses are: 

veth3 - a2:5e:37:ac:a1:7f

veth0 - fa:4f:e8:df:b1:5f

	$ scapy
	>>> pkt2 = Ether(dst='a2:5e:37:ac:a1:7f',src='fa:4f:e8:df:b1:5f')/IP(dst='192.168.0.2',src='192.168.0.1')
	>>> sendp(pkt2,iface="veth0",count=1);

The first packet with an unknown destination mac address will be broadcasted by the switch while the source mac address is learned. Now after the two packets were sent, the switch has already learned the mac addresses of veth0 and veth3. Now if we send those packets again, switch will forward those packets via corresponding ports instead of broadcasting them.

## Usecases

Other tested P4 usecases with its dependency graphs can be found at [macsad-usecases](https://github.com/intrig-unicamp/macsad-usecases) repository.

## Acknowledgment

This work was supported by the Innovation Center, Ericsson Telecomunicações S.A., Brazil under grant agreement UNI.61.
