MACSAD
==========

Downloading the MACSAD project:

`git clone git@github.com:intrig-unicamp/mac.git`

Updating the submodules:

- `cd mac`
- `git submodule init`
- `git submodule update`

#P4-Hlir

After download the submodules, is necessary install the P4-Hlir, follow the steps below:

Dependencies:

The following are required to run `p4-validate` and `p4-graphs`:

- the Python `yaml` package
- the Python `ply` package
- the `dot` tool

`ply` will be installed automatically by `setup.py` when installing p4-hlir.

On Ubuntu, the following packages can be installed with apt-get to satisfy the remaining dependencies:

- `python-yaml`
- `graphviz`

To install:

`sudo python setup.py install`

To run validate tool:

p4-validate \<path_to_p4_program\>

To open a Python shell with an HLIR instance accessible:

p4-shell \<path_to_p4_program\>

#ODP:

Is necessary download and compile the ODP project, follow the steps below:

`git clone https://git.linaro.org/lng/odp.git`

`cd odp`

- `./bootstrap`
- `./configure`
- `make`

After install ODP, is necessary to set the environment variable `ODP_SDK` with the installation directory `~/odp`

export ODP_SDK=\<path_of_ODP\>

Also can be added as a environment variable at the `~/.bashrc`file

#Compiling MACSAD

Inside our `mac` directory. First we need to add the P4 c files with the command:

`python src/compiler.py examples/p4_src/l2_switch_test.p4`

*This line need to be run if the `compiler.py` is modified or if any file inside `src/hardware_indep` has been changed.

Then we can compile the project with:

`make`

#Running an example

TERMINAL 1

Start the Controller:

- `cd src/hardware_dep/shared/ctrl_plane`
- `make clean`
- `make mac_controller`
- `./mac_controller`

TERMINAL 2

Create veth interfaces:

- `cd /script`
- `./host_create_veth_interfaces.sh`

Set the environment variable `LD_LIBRARY_PATH` with the directory `~/odp/lib/.libs`

export LD_LIBRARY_PATH=\<path_of_odp_lib_.libs\>

Start the switch

Run the switch with veth interfaces 1.0 and 2.1

- `./mac_ad -i veth1.0,veth2.1`

*Run as root

TERMINAL 3

Using scapy, we can create packages and send it from one interface to other

`sudo apt-get install scapy`

Scapy is necessary to run as root

`sudo scapy`

The first package goes from veth2.0 to veth1.1

Is necessary to use the MAC address of your veth2.0 and veth1.1 in order to create the package. For this example the MAC address are going to be: veth2.0 a2:5e:37:ac:a1:7f and veth1.1 fa:4f:e8:df:b1:5f.

`pkt = Ether(dst='fa:4f:e8:df:b1:5f',src='a2:5e:37:ac:a1:7f')/IP(dst='192.168.0.1',src='192.168.0.2')`

To send the package from veth2.0 as source, we are going to excecute the command below:

`sendp(pkt,iface="veth2.0",count=1);`

*The complete process of sending the package can be seen at TERMINAL 2

Now we are going to send a package from veth1.1 to veth2.0:

`pkt = Ether(dst='a2:5e:37:ac:a1:7f',src='fa:4f:e8:df:b1:5f')/IP(dst='192.168.0.1',src='192.168.0.2')`

`sendp(pkt,iface="veth1.1",count=1);`





