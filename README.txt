
You can execute the program the following way.
These steps turn off features which are semi-complete at the moment.
(As the code improves, this description is expected to become deprecated very soon.)

1. At the end of compiler.py, you'll find the main method.
   Comment out the last lines that concern data plane analysis and the parser generator.

	# init_data_plane_analyser(p4_path)
	# analyse_data_plane()

	# generate_parser_file(p4_path, hlir)

2. In Makefile, comment out the two lines that are related to the above features.

	# SRCS-y += data_plane_data.c
	# SRCS-y += parser.c

With these modifications, both test_dpdk1.sh and test_pcap1.sh should generate files into build/ without errors (there will be a couple of warnings, though).
