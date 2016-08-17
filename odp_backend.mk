CDIR := $(dir $(lastword $(MAKEFILE_LIST)))

VPATH += $(CDIR)/src/hardware_dep/odp/
VPATH += $(CDIR)/src/hardware_dep/odp/includes
VPATH += $(CDIR)/src/hardware_dep/odp/ctrl_plane
VPATH += $(CDIR)/src/hardware_dep/odp/data_plane

VPATH += $(ODP_SDK)/include
VPATH += $(ODP_SDK)/platform/linux-generic/include
VPATH += $(ODP_SDK)/platform/linux-generic/arch/x86
VPATH += $(ODP_SDK)/share
VPATH += $(ODP_SDK)/helper/include/odp/helper

CFLAGS += -I "$(CDIR)/src/hardware_dep/odp/includes"
CFLAGS += -I "$(CDIR)/src/hardware_dep/odp/ctrl_plane"
CFLAGS += -I "$(CDIR)/src/hardware_dep/odp/data_plane"

#ODP APIs
CFLAGS += -I "$(ODP_SDK)/include"
CFLAGS += -I "$(ODP_SDK)/platform/linux-generic/include"
CFLAGS += -I "$(ODP_SDK)/platform/linux-generic/arch/x86"
CFLAGS += -I "$(ODP_SDK)/share"

#ODP Helper APIs
CFLAGS += -I "$(ODP_SDK)/helper/include"
CFLAGS += -I "$(ODP_SDK)/helper"
CFLAGS += -I "$(ODP_SDK)/helper/include/odp"
CFLAGS += -I "$(ODP_SDK)/helper/include/odp/helper"

# odp main
SRCS-y += $(CDIR)/src/hardware_dep/odp/main.c
SRCS-y += $(CDIR)/src/hardware_dep/odp/main_odp.c

# control plane related sources
SRCS-y += $(CDIR)/src/hardware_dep/shared/ctrl_plane/ctrl_plane_backend.c
SRCS-y += $(CDIR)/src/hardware_dep/shared/ctrl_plane/fifo.c
SRCS-y += $(CDIR)/src/hardware_dep/shared/ctrl_plane/handlers.c
SRCS-y += $(CDIR)/src/hardware_dep/shared/ctrl_plane/messages.c
SRCS-y += $(CDIR)/src/hardware_dep/shared/ctrl_plane/sock_helpers.c
SRCS-y += $(CDIR)/src/hardware_dep/shared/ctrl_plane/threadpool.c

# data plane related includes
SRCS-y += $(CDIR)/src/hardware_dep/odp/data_plane/odp_lib.c
SRCS-y += $(CDIR)/src/hardware_dep/odp/data_plane/odp_tables.c
SRCS-y += $(CDIR)/src/hardware_dep/odp/data_plane/odp_primitives.c
#SRCS-y += odp_tables.c
#SRCS-y += odp_primitives.c
SRCS-Y += vector.c

#CC = clang

LDFLAGS += -L$(ODP_SDK)/lib/
LDFLAGS += -L$(ODP_SDK)/lib/.libs
LIBS = -lodp-linux -lodphelper-linux -lpthread
#LIBS = -lodp-dpdk -lodphelper-linux -lpthread

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

# DO NOT DELETE THIS LINE
