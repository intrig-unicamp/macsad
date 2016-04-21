MACSAD
==========

Downloading the MACSAD project:

`git clone git@github.com:intrig-unicamp/mac.git`

Updating the submodules:

`cd mac`
`git submodule init`
`git submodule update`

#P4-Hlir

After download the submodules, is necessary intall the P4-Hlir, follow the steps below:

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

Is necessary download and compyle the ODP project, follow the stepts below:

`git clone https://git.linaro.org/lng/odp.git`

`cd odp`

- `./bootstrap`
- `./configure`
- `make`

After install ODP, is necessary to set the enviroment variable ODP_SDK with the instalation directory `~/odp`

export ODP_SDK=\<path_of_ODP\>

Also can be added as a enviroment variable at the `~/Bashrc`file

#Compiling MACSAD

Inside our `mac` directory. First we nedd to add the P4 c files with the command:

`python src/compiler.py examples/p4_src/l2_switch_test.p4`

Then we can compile the project with:

`make`

#Running and example


