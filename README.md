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

After install ODP, is necessary to set the environment variable ODP_SDK with the installation directory `~/odp`

export ODP_SDK=\<path_of_ODP\>

Also can be added as a environment variable at the `~/Bashrc`file

#Compiling MACSAD

Inside our `mac` directory. First we need to add the P4 c files with the command:

`python src/compiler.py examples/p4_src/l2_switch_test.p4`

*This line need to be run if the `compyle.py` is modified or if any file inside `src/hardware_indep` has been changed.

Then we can compile the project with:

`make`

#Running an example



