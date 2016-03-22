CDIR := $(dir $(lastword $(MAKEFILE_LIST)))

VPATH += $(CDIR)/src/hardware_dep/dpdk/
VPATH += $(CDIR)/src/hardware_dep/dpdk/includes
VPATH += $(CDIR)/src/hardware_dep/dpdk/ctrl_plane
VPATH += $(CDIR)/src/hardware_dep/dpdk/data_plane

# dpdk main
SRCS-y += main.c
SRCS-y += main_dpdk_main_loop.c

# control plane related sources
SRCS-y += ctrl_plane_backend.c
SRCS-y += fifo.c
SRCS-y += handlers.c
SRCS-y += messages.c
SRCS-y += sock_helpers.c
SRCS-y += threadpool.c

# data plane related includes
SRCS-y += dpdk_lib.c
SRCS-y += dpdk_tables.c
SRCS-y += dpdk_primitives.c
SRCS-y += ternary_naive.c

CFLAGS += -I "$(CDIR)/src/hardware_dep/dpdk/includes"
CFLAGS += -I "$(CDIR)/src/hardware_dep/dpdk/ctrl_plane"
CFLAGS += -I "$(CDIR)/src/hardware_dep/dpdk/data_plane"
