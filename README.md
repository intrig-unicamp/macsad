# MACSAD

[Intro: what is MACSAD? why does it exist? how does it work?]

To run MACSAD, follow the steps below. We have tested on Ubuntu 16.04.

## ODP installation

MACSAD uses ODP for forwarding plane developement.

1. Download ODP v1.16.0, compile and install it:

        sudo apt-get install build-essential autoconf automake pkg-config libssl-dev
        wget https://github.com/Linaro/odp/archive/v1.16.0.0.tar.gz
        tar xzvf v1.16.0.0.tar.gz
        mv odp-1.16.0.0/ odp/
        cd odp-1.16.0.0/
        ./bootstrap
        ./configure --disable-abi-compat --prefix=`pwd`/build
        make
        make install

2. Set the library and pkg-config info for ODP (make sure you are at the root
of the ODP build directory before running any of these commands):
        echo `pwd`/build/lib | sudo tee /etc/ld.so.conf.d/odp.conf
        sudo ldconfig
        export PKG_CONFIG_PATH=`pwd`/build/lib/pkgconfig
        echo "export PKG_CONFIG_PATH=`pwd`/build/lib/pkgconfig" >> ~/.bashrc
        cd ..

## MACSAD installation

1. Clone the MACSAD project:

        git clone https://github.com/intrig-unicamp/mac.git
        cd mac

2. MACSAD has added `P4-hlir` as a submodule. Download/update and then install
it (along with its dependencies):

        git submodule update --init --recursive
        sudo apt-get install python-yaml graphviz python-setuptools
        cd p4-hlir
        python setup.py install --user

    For any issues with p4-hlir, please refer to its `README.md` file.

3. Translate the P4 program to MACSAD:

        python src/transpiler.py examples/p4_src/l2_fwd.p4

4. Install build dependencies, project dependencies and compile MACSAD:

        sudo apt-get install autoconf libtool build-essential pkg-config
        sudo apt-get install libpcap-dev python-scapy
        ./autogen.sh
        ./configure
        make

5. Set hugepages number and create interfaces for the test:

        sudo sysctl -w vm.nr_hugepages=512
        sudo ./scripts/veth_create.sh

6. Start the minimalistic controller (required for installing flows in the
switch):

        ./ctrl/mac_controller

    The controller will run in the foreground, so feel free to open another
    screen or terminal session.

7. Run the MACSAD switch:

        sudo ./macsad -i veth1,veth2 -c 0 -m 0 --out_mode 0

8. Now will use the veth interfaces created in step #5. `veth1` and `veth2` will
be part of the switch. We will send packet to `veth0` (which is `veth1`'s pair)
and monitor at `veth3` (which is `veth2`'s pair) for packets. Similary we will
send packet to `veth3` and expect packets to arrive at `veth0`:

        sudo python run_test.py

    You should see output similar to this:

        $ sudo python run_test.py
        WARNING: No route found for IPv6 destination :: (no default route?)
        Sending 1 packets to  veth3 ...
        DISTRIBUTION:
        port veth0:   1 [ 100.0% ]
        port veth3:   1 [ 100.0% ]
