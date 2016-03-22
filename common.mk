
ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

ifeq ($(ODP_SDK),)
$(error "Please define ODP_SDK environment variable")
endif

# Default target, can be overriden by command line or environment
#RTE_TARGET ?= x86_64-native-linuxapp-gcc

#include $(RTE_SDK)/mk/rte.vars.mk

# Compiler debug level
CFLAGS += -O3
CFLAGS += -g3

CFLAGS += -Wall 
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-unused-value

CFLAGS += -std=gnu99

#To define the backend used: odp or dpdk
CFLAGS += -D ODP_BK
#CFLAGS += -D ODP_BK

#CFLAGS += -always-make
