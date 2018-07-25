# Copyright 2018 INTRIG/FEEC/UNICAMP (University of Campinas), Brazil
#
#Licensed under the Apache License, Version 2.0 (the "License");
#you may not use this file except in compliance with the License.
#You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#Unless required by applicable law or agreed to in writing, software
#distributed under the License is distributed on an "AS IS" BASIS,
#WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#See the License for the specific language governing permissions and
#limitations under the License.

#CDIR := $(dir $(lastword $(MAKEFILE_LIST)))
CDIR := ../

VPATH += $(CDIR)/src/hardware_dep/odp
VPATH += $(CDIR)/src/hardware_dep/odp/includes
VPATH += $(CDIR)/src/hardware_dep/odp/ctrl_plane
VPATH += $(CDIR)/src/hardware_dep/odp/data_plane

VPATH += $(ODP_SDK)/share
VPATH += $(ODP_SDK)/include
VPATH += $(ODP_SDK)/helper/include/odp/helper
VPATH += $(ODP_SDK)/platform/linux-generic/include
VPATH += $(ODP_SDK)/platform/linux-generic/arch/x86
VPATH += $(ODP_SDK)/platform/linux-generic/include/odp/api/plat
#VPATH += $(ODP_SDK)/platform/linux-dpdk/include
#VPATH += $(ODP_SDK)/platform/linux-dpdk/arch/x86
#VPATH += $(ODP_SDK)/platform/linux-dpdk/include/odp/api/plat

CFLAGS += -I "$(CDIR)src/hardware_dep/odp/includes"
CFLAGS += -I "$(CDIR)src/hardware_dep/odp/ctrl_plane"
CFLAGS += -I "$(CDIR)src/hardware_dep/odp/data_plane"
#ODP APIs
CFLAGS += -I "$(ODP_SDK)/share"
CFLAGS += -I "$(ODP_SDK)/include"
CFLAGS += -I "$(ODP_SDK)/platform/linux-generic/include"
CFLAGS += -I "$(ODP_SDK)/platform/linux-generic/arch/x86"
CFLAGS += -I "$(ODP_SDK)/platform/linux-generic/include/odp/api/plat"
#CFLAGS += -I "$(ODP_SDK)/platform/linux-dpdk/include"
#CFLAGS += -I "$(ODP_SDK)/platform/linux-dpdk/arch/x86"
#CFLAGS += -I "$(ODP_SDK)/platform/linux-dpdk/include/odp/api/plat"

#ODP Helper APIs
CFLAGS += -I "$(ODP_SDK)/helper"
CFLAGS += -I "$(ODP_SDK)/helper/include"
CFLAGS += -I "$(ODP_SDK)/helper/include/odp/helper"

# odp main
SRCS-y += $(CDIR)src/hardware_dep/odp/main.c
SRCS-y += $(CDIR)src/hardware_dep/odp/main_odp.c

# control plane related sources
SRCS-y += $(CDIR)src/hardware_dep/shared/ctrl_plane/ctrl_plane_backend.c
SRCS-y += $(CDIR)src/hardware_dep/shared/ctrl_plane/fifo.c
SRCS-y += $(CDIR)src/hardware_dep/shared/ctrl_plane/handlers.c
SRCS-y += $(CDIR)src/hardware_dep/shared/ctrl_plane/messages.c
SRCS-y += $(CDIR)src/hardware_dep/shared/ctrl_plane/sock_helpers.c
SRCS-y += $(CDIR)src/hardware_dep/shared/ctrl_plane/threadpool.c

# data plane related includes
SRCS-y += $(CDIR)src/hardware_dep/odp/data_plane/odp_lib.c
SRCS-y += $(CDIR)src/hardware_dep/odp/data_plane/odp_tables.c
SRCS-y += $(CDIR)src/hardware_dep/odp/data_plane/odp_primitives.c
SRCS-Y += vector.c

LDFLAGS += -L$(ODP_SDK)/lib


#01 ODP(socket-mmap, netmap)
##static
#ODP_LIB = -l:libodp-linux.a -lcrypto -ldl -lpcap -lrt -lpthread -latomic -lconfig
#HELPER_LIB = -l:libodphelper.a

##shared
ODP_LIB = -lodp-linux -lcrypto -ldl -lpcap -lrt -lpthread -latomic -lconfig
HELPER_LIB = -lodphelper


#02 ODP(dpdk)
#LDFLAGS += -L$(RTE_SDK)/x86_64-native-linuxapp-gcc/lib
##static
#ODP_LIB = -l:libodp-linux.a -lcrypto -ldl -Wl,--whole-archive,-lrte_pmd_af_packet,-lrte_pmd_ark,-lrte_pmd_avp,-lrte_pmd_bnxt,-lrte_pmd_bond,-lrte_pmd_crypto_scheduler,-lrte_pmd_cxgbe,-lrte_pmd_e1000,-lrte_pmd_ena,-lrte_pmd_enic,-lrte_pmd_failsafe,-lrte_pmd_fm10k,-lrte_pmd_i40e,-lrte_pmd_ixgbe,-lrte_pmd_kni,-lrte_pmd_lio,-lrte_pmd_nfp,-lrte_pmd_null,-lrte_pmd_null_crypto,-lrte_pmd_octeontx,-lrte_pmd_octeontx_ssovf,-lrte_pmd_pcap,-lrte_pmd_qede,-lrte_pmd_ring,-lrte_pmd_sfc_efx,-lrte_pmd_skeleton_event,-lrte_pmd_softnic,-lrte_pmd_sw_event,-lrte_pmd_tap,-lrte_pmd_thunderx_nicvf,-lrte_pmd_vhost,-lrte_pmd_virtio,-lrte_pmd_vmxnet3_uio,--no-whole-archive -ldpdk -ldl -lpthread -lnuma -lm -lpcap -lrt -lpthread -latomic -lconfig
#HELPER_LIB = -l:libodphelper.a

##shared
#LDFLAGS += -L$(RTE_SDK)/x86_64-native-linuxapp-gcc/lib
#ODP_LIB = -lodp-linux -lcrypto -ldl -ldpdk -ldl -lpthread -lnuma -lm -lpcap -lrt -lpthread -latomic -lconfig
#HELPER_LIB = -lodphelper
##Experimental
#LIBS =  `PKG_CONFIG_PATH=/root/gyn/odp/buildD/lib/pkgconfig:${PKG_CONFIG_PATH} pkg-config --cflags --libs libodp-linux libodphelper --static`

#03 ODP-DPDK
#CFLAGS  += -I "$(RTE_SDK)/$(RTE_TARGET)/include"
#LDFLAGS += -L$(RTE_SDK)/x86_64-native-linuxapp-gcc/lib
#Need to use shared library to compile with hash table support.
##static
#ODP_LIB = -l:libodp-dpdk.a -lpthread -lrt -lcrypto -ldl -lpcap -Wl,--whole-archive,-ldpdk,--no-whole-archive -ldl -lm -lpcap
#HELPER_LIB = -l:libodphelper.a

##shared
#ODP_LIB = -lodp-dpdk -lpthread -lrt -lcrypto -ldl -lpcap -Wl,--whole-archive,-ldpdk,--no-whole-archive -ldl -lm -lpcap
#HELPER_LIB = -lodphelper


LIBS = $(ODP_LIB) $(HELPER_LIB)

OBJS = $(SRCS-y:.c=.o)

MAIN = macsad

all: $(MAIN)
	@echo  $(MAIN) successfully compiled

$(MAIN):
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $(MAIN) $(SRCS-y) $(LIBS) 2>/dev/null
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(MAIN) $(SRCS-y) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $^

###Custom Targets
tags:
	ctags -R --languages=C,C++ --exclude=.git,scripts,tools

# DO NOT DELETE THIS LINE
