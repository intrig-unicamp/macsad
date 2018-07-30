bin_PROGRAMS = macsad ctrl/mac_controller ctrl/mac_l2_l3_controller ctrl/mac_l3_controller ctrl/mac_l3_nhg_controller

macsad_SOURCES = build/src_hardware_indep/dataplane.c \
    build/src_hardware_indep/tables.c \
    build/src_hardware_indep/parser.c \
    build/src_hardware_indep/actions.c \
    build/src_hardware_indep/controlplane.c \
    src/hardware_dep/odp/main.c \
    src/hardware_dep/odp/main_odp.c \
    src/hardware_dep/shared/ctrl_plane/ctrl_plane_backend.c \
    src/hardware_dep/shared/ctrl_plane/fifo.c \
    src/hardware_dep/shared/ctrl_plane/handlers.c \
    src/hardware_dep/shared/ctrl_plane/messages.c \
    src/hardware_dep/shared/ctrl_plane/sock_helpers.c \
    src/hardware_dep/shared/ctrl_plane/threadpool.c \
    src/hardware_dep/odp/data_plane/odp_lib.c \
    src/hardware_dep/odp/data_plane/odp_tables.c \
    src/hardware_dep/odp/data_plane/odp_primitives.c

ctrl_mac_controller_SOURCES = \
    src/hardware_dep/shared/ctrl_plane/handlers.c \
    src/hardware_dep/shared/ctrl_plane/controller.c \
    src/hardware_dep/shared/ctrl_plane/messages.c \
    src/hardware_dep/shared/ctrl_plane/sock_helpers.c \
    src/hardware_dep/shared/ctrl_plane/threadpool.c \
    src/hardware_dep/shared/ctrl_plane/fifo.c \
    src/hardware_dep/shared/ctrl_plane/mac_controller.c

ctrl_mac_l2_l3_controller_SOURCES = \
    src/hardware_dep/shared/ctrl_plane/handlers.c \
    src/hardware_dep/shared/ctrl_plane/controller.c \
    src/hardware_dep/shared/ctrl_plane/messages.c \
    src/hardware_dep/shared/ctrl_plane/sock_helpers.c \
    src/hardware_dep/shared/ctrl_plane/threadpool.c \
    src/hardware_dep/shared/ctrl_plane/fifo.c \
    src/hardware_dep/shared/ctrl_plane/mac_l2_l3_controller.c

ctrl_mac_l3_controller_SOURCES = \
    src/hardware_dep/shared/ctrl_plane/handlers.c \
    src/hardware_dep/shared/ctrl_plane/controller.c \
    src/hardware_dep/shared/ctrl_plane/messages.c \
    src/hardware_dep/shared/ctrl_plane/sock_helpers.c \
    src/hardware_dep/shared/ctrl_plane/threadpool.c \
    src/hardware_dep/shared/ctrl_plane/fifo.c \
    src/hardware_dep/shared/ctrl_plane/mac_l3_controller.c

ctrl_mac_l3_nhg_controller_SOURCES = \
    src/hardware_dep/shared/ctrl_plane/handlers.c \
    src/hardware_dep/shared/ctrl_plane/controller.c \
    src/hardware_dep/shared/ctrl_plane/messages.c \
    src/hardware_dep/shared/ctrl_plane/sock_helpers.c \
    src/hardware_dep/shared/ctrl_plane/threadpool.c \
    src/hardware_dep/shared/ctrl_plane/fifo.c \
    src/hardware_dep/shared/ctrl_plane/mac_l3_nhg_controller.c

ODP_CFLAGS = $(libodp_CFLAGS) $(libodphelper_CFLAGS)
AM_CFLAGS = $(ODP_CFLAGS) $(PTHREAD_CFLAGS) -D NDEBUG -D NINFO -D NSIGG -D NERROR -D NWARN -D CUCKOO

ODP_LIBS = $(libodp_LIBS) $(libodphelper_LIBS)
LIBS += $(ODP_LIBS)  $(PTHREAD_LIBS) 

if MAC_ODP_DPDK
AM_CFLAGS  += -I "$(RTE_SDK)/$(RTE_TARGET)/include"
AM_LDFLAGS = -L$(RTE_SDK)/$(RTE_TARGET)/lib
#AM_LDFLAGS = $(DPDK_LDFLAGS)
#AM_CFLAGS += $(DPDK_CFLAGS)
LIBS += $(DPDK_LIBS)
endif

if MAC_ODPD
AM_CFLAGS  += -I "$(RTE_SDK)/$(RTE_TARGET)/include"
AM_LDFLAGS = -L$(RTE_SDK)/$(RTE_TARGET)/lib
LIBS += $(DPDK_LIBS)
endif

if MAC_ODPN
LIBS += -lrt -lcrypto -lpcap
endif

#**********************

# FIXME: This should not be necessary: files should have relative references to other files in the project
AM_CPPFLAGS = -I "src/hardware_dep/shared/includes" -I "src/hardware_dep/shared/ctrl_plane" -I "build/src_hardware_indep" -I "src/hardware_dep/odp/includes"
