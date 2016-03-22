ifeq ($(ODP_SDK),)
$(error "Please define ODP_SDK environment variable")
endif

# binary name
APP = example_odp1

#include $(SRCDIR)/dpdk_backend.mk
include common.mk
include hw_independent.mk
include odp_backend.mk

