CDIR := $(dir $(lastword $(MAKEFILE_LIST)))

# the directories of the source files
VPATH += $(CDIR)/build/src_hardware_indep

VPATH += $(CDIR)/src/hardware_dep/shared/
VPATH += $(CDIR)/src/hardware_dep/shared/includes
VPATH += $(CDIR)/src/hardware_dep/shared/ctrl_plane
VPATH += $(CDIR)/src/hardware_dep/shared/data_plane

# the names of the source files
SRCS-y += $(CDIR)build/src_hardware_indep/dataplane.c
SRCS-y += $(CDIR)build/src_hardware_indep/tables.c
SRCS-y += $(CDIR)build/src_hardware_indep/parser.c
SRCS-y += $(CDIR)build/src_hardware_indep/actions.c
SRCS-y += $(CDIR)build/src_hardware_indep/controlplane.c
#SRCS-y += dataplane.c
#SRCS-y += tables.c
#SRCS-y += parser.c
#SRCS-y += actions.c
#SRCS-y += controlplane.c

CFLAGS += -I "$(CDIR)src/hardware_dep/shared/includes"
CFLAGS += -I "$(CDIR)src/hardware_dep/shared/ctrl_plane"
CFLAGS += -I "$(CDIR)src/hardware_dep/shared/data_plane"

CFLAGS += -I "$(CDIR)build/src_hardware_indep"
