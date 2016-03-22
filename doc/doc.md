
# Required software

- [VirtualBox 5.0.6](https://www.virtualbox.org/wiki/Downloads)
- [DPDK 2.1.0](http://dpdk.org/download)
- [Linux Mint 17.2](http://www.linuxmint.com/edition.php?id=192)

Similar software (e.g. other Linux distributions) and older versions (e.g. v4 of VirtualBox) will probably work, too.


# Installation

This description is applicable to both Windows and Linux hosts except where noted.

1. Make a VirtualBox virtual machine, and install Linux on it.
1. Setup the virtual machine environment
	1. `VirtualBox->File->Preferences->Network->Host-only Networks`, add three adapters
	1. `(your virtual machine)->Settings->System->Processor`, set at least two CPUs.
	1. `(your virtual machine)->Settings->System->Network`, enable `Adapter 2`, `Adapter 3` and `Adapter 4`, and for each of them,
		a. Under `Advanced`, set `Adapter Type` to `Intel PRO/1000 MT Server (82545EM)`
		a. Under `Attached to`, set `Host-only Adapter`, and `Name` to those of the adapters


## Configuration of the guest system

1. Install necessary/useful Linux packages: `sudo aptitude install g++-multilib libvirt-dev python-setuptools clang-format-3.6 libpcap-dev pandoc subversion git`
1. Setup DPDK
	1. setup exports
		- e.g. you can insert the below code into `~/.bashrc`
		- the scripts use the following variables (all have sensible defaults)
			- `ARCH`: the target architecture
			- `MAKE_CMD`: the make command to use
			- `P4DPDK_OPTS`: the EAL options that are given to the DPDK program
				- `${P4DPDK_TRUNK}/scripts/set_opts_*` are some handy scripts, the following example uses one of them
				- `set_opts_default` gives you a performant DPDK environment, `_nohuge` is a debug option that doesn't use hugefiles, and `_terse` suppresses output

		~~~~~~~
		export RTE_SDK=insert/your/path/to/dpdk/here
		export RTE_TARGET=x86_64-native-linuxapp-gcc

		export P4DPDK_TRUNK=insert/your/path/to/p4dpdk/trunk/here

		export MAKE_CMD=${P4DPDK_TRUNK}/scripts/colorized-make.sh
		. ${P4DPDK_TRUNK}/scripts/set_opts_nohuge_terse.sh
		~~~~~~~

	1. run `${P4DPDK_TRUNK}/scripts/guest_preinstall_run_once.sh`
1. Setup P4
	1. `git clone https://github.com/p4lang/p4-hlir`
	1. inside the just created `p4-hlir` directory, run `sudo python setup.py install`


### Steps at the start of the day

Each time before you start working, you'll have to prepare your system.
Run `${P4DPDK_TRUNK}/scripts/guest_preinstall_run_once.sh`, which will configure your system as needed.

These settings are cleared when you turn off your machine,
or you can use `${P4DPDK_TRUNK}/scripts/guest_dpdk_off.sh` to explicitly clean up after you have run your DPDK programs.


# Trying DPDK Examples

1. In `$RTE_SDK`, run `sudo make examples -j16 T=x86_64-native-linuxapp-gcc`
1. Run `${P4DPDK_TRUNK}/scripts/dpdk_example_bridge.sh`
	- it will run a bridge, and feed it some data (from the guest, through Adapter 3)
1. Run `${P4DPDK_TRUNK}/scripts/dpdk_example_l2fwd.sh`
	- it will display a "Port statistics" table
1. Run `${P4DPDK_TRUNK}/scripts/dpdk_example_l3fwd.sh`
	- it will start and not fail
	- it should particularly not show any `err=-22` error message

TODO describe how to send packets that the examples can get and handle


# Running the P4 example

1. go into the `trunk` directory
2. run `./test_dpdk1.sh`
	- it executes `make`
	- it executes the built program as root, with appropriate parameters

TODO make a tester that
- runs the program
- sends packets to the emulated ports
- collects the forwarded packets
- checks if port forwarding is working all right

NOTE: `test_pcap1.sh` doesn't work temporarily, as it is not up to date


# The structure of the compiler

In the following, `$ARCH` refers to one of the supported architectures, e.g. `pcap` or `dpdk`.

- `src/`: the compiler and its utilities
- `src/hardware_indep/`: code generators for hardware independent parts
	- `src/hardware_indep/parser`: code generator for the packet parser
- `src/hardware_dep/$ARCH`: the hardware dependent parts
	- `src/hardware_dep/$ARCH/includes`: headers
	- `src/hardware_dep/$ARCH/data_plane`: code for the data plane
	- `src/hardware_dep/$ARCH/ctrl_plane`: code for the control plane

