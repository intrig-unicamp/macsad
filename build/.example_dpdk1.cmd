cmd_example_dpdk1 = gcc -m64 -pthread  -march=native -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX -DRTE_MACHINE_CPUFLAG_RDRAND -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX -DRTE_MACHINE_CPUFLAG_RDRAND -DRTE_COMPILE_TIME_CPUFLAGS=RTE_CPUFLAG_SSE,RTE_CPUFLAG_SSE2,RTE_CPUFLAG_SSE3,RTE_CPUFLAG_SSSE3,RTE_CPUFLAG_SSE4_1,RTE_CPUFLAG_SSE4_2,RTE_CPUFLAG_AES,RTE_CPUFLAG_PCLMULQDQ,RTE_CPUFLAG_AVX,RTE_CPUFLAG_RDRAND,RTE_CPUFLAG_SSE,RTE_CPUFLAG_SSE2,RTE_CPUFLAG_SSE3,RTE_CPUFLAG_SSSE3,RTE_CPUFLAG_SSE4_1,RTE_CPUFLAG_SSE4_2,RTE_CPUFLAG_AES,RTE_CPUFLAG_PCLMULQDQ,RTE_CPUFLAG_AVX,RTE_CPUFLAG_RDRAND  -I/home/ubuntu/p4eric/repos/trunk/build/include -I/home/ubuntu/p4eric/dpdk/dpdk-2.1.0//x86_64-native-linuxapp-gcc/include -include /home/ubuntu/p4eric/dpdk/dpdk-2.1.0//x86_64-native-linuxapp-gcc/include/rte_config.h -O3 -Wall  -Wno-unused-function -g -std=gnu99 -I /home/ubuntu/p4eric/repos/trunk//src/hardware_dep/dpdk/includes -I /home/ubuntu/p4eric/repos/trunk//src/hardware_dep/dpdk/ctrl_plane -I /home/ubuntu/p4eric/repos/trunk//src/hardware_dep/dpdk/data_plane -I /home/ubuntu/p4eric/repos/trunk//src/hardware_dep/shared/includes -I /home/ubuntu/p4eric/repos/trunk//src/hardware_dep/shared/ctrl_plane -I /home/ubuntu/p4eric/repos/trunk//src/hardware_dep/shared/data_plane -I /home/ubuntu/p4eric/repos/trunk//build/src_hardware_indep  -Wl,-Map=example_dpdk1.map,--cref -o example_dpdk1 main.o main_dpdk_main_loop.o ctrl_plane_backend.o fifo.o handlers.o messages.o sock_helpers.o threadpool.o dpdk_lib.o dpdk_tables.o dpdk_primitives.o ternary_naive.o dataplane.o tables.o parser.o actions.o controlplane.o -Wl,--no-as-needed -Wl,-export-dynamic -L/home/ubuntu/p4eric/repos/trunk/build/lib -L/home/ubuntu/p4eric/dpdk/dpdk-2.1.0//x86_64-native-linuxapp-gcc/lib  -L/home/ubuntu/p4eric/dpdk/dpdk-2.1.0//x86_64-native-linuxapp-gcc/lib -Wl,--whole-archive -Wl,-lrte_distributor -Wl,-lrte_reorder -Wl,-lrte_kni -Wl,-lrte_pipeline -Wl,-lrte_table -Wl,-lrte_port -Wl,-lrte_timer -Wl,-lrte_hash -Wl,-lrte_jobstats -Wl,-lrte_lpm -Wl,-lrte_power -Wl,-lrte_acl -Wl,-lrte_meter -Wl,-lrte_sched -Wl,-lm -Wl,-lrt -Wl,-lrte_vhost -Wl,--start-group -Wl,-lrte_kvargs -Wl,-lrte_mbuf -Wl,-lrte_ip_frag -Wl,-lethdev -Wl,-lrte_mempool -Wl,-lrte_ring -Wl,-lrte_eal -Wl,-lrte_cmdline -Wl,-lrte_cfgfile -Wl,-lrte_pmd_bond -Wl,-lrte_pmd_vmxnet3_uio -Wl,-lrte_pmd_virtio -Wl,-lrte_pmd_cxgbe -Wl,-lrte_pmd_enic -Wl,-lrte_pmd_i40e -Wl,-lrte_pmd_fm10k -Wl,-lrte_pmd_ixgbe -Wl,-lrte_pmd_e1000 -Wl,-lrte_pmd_ring -Wl,-lrte_pmd_af_packet -Wl,-lrte_pmd_null -Wl,-lrt -Wl,-lm -Wl,-ldl -Wl,--end-group -Wl,--no-whole-archive 
