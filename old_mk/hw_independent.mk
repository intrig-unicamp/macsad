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
