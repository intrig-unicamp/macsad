cmd_main_dpdk_main_loop.o = gcc -Wp,-MD,./.main_dpdk_main_loop.o.d.tmp -m64 -pthread  -march=native -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX -DRTE_MACHINE_CPUFLAG_RDRAND -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX -DRTE_MACHINE_CPUFLAG_RDRAND -DRTE_COMPILE_TIME_CPUFLAGS=RTE_CPUFLAG_SSE,RTE_CPUFLAG_SSE2,RTE_CPUFLAG_SSE3,RTE_CPUFLAG_SSSE3,RTE_CPUFLAG_SSE4_1,RTE_CPUFLAG_SSE4_2,RTE_CPUFLAG_AES,RTE_CPUFLAG_PCLMULQDQ,RTE_CPUFLAG_AVX,RTE_CPUFLAG_RDRAND,RTE_CPUFLAG_SSE,RTE_CPUFLAG_SSE2,RTE_CPUFLAG_SSE3,RTE_CPUFLAG_SSSE3,RTE_CPUFLAG_SSE4_1,RTE_CPUFLAG_SSE4_2,RTE_CPUFLAG_AES,RTE_CPUFLAG_PCLMULQDQ,RTE_CPUFLAG_AVX,RTE_CPUFLAG_RDRAND  -I/home/ubuntu/p4eric/repos/trunk/build/include -I/home/ubuntu/p4eric/dpdk/dpdk-2.1.0//x86_64-native-linuxapp-gcc/include -include /home/ubuntu/p4eric/dpdk/dpdk-2.1.0//x86_64-native-linuxapp-gcc/include/rte_config.h -O3 -Wall  -Wno-unused-function -g -std=gnu99 -I "/home/ubuntu/p4eric/repos/trunk//src/hardware_dep/dpdk/includes" -I "/home/ubuntu/p4eric/repos/trunk//src/hardware_dep/dpdk/ctrl_plane" -I "/home/ubuntu/p4eric/repos/trunk//src/hardware_dep/dpdk/data_plane" -I "/home/ubuntu/p4eric/repos/trunk//src/hardware_dep/shared/includes" -I "/home/ubuntu/p4eric/repos/trunk//src/hardware_dep/shared/ctrl_plane" -I "/home/ubuntu/p4eric/repos/trunk//src/hardware_dep/shared/data_plane" -I "/home/ubuntu/p4eric/repos/trunk//build/src_hardware_indep"   -o main_dpdk_main_loop.o -c /home/ubuntu/p4eric/repos/trunk//src/hardware_dep/dpdk/main_dpdk_main_loop.c 
