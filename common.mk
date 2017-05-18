ifeq ($(ODP_SDK),)
$(error "Please define ODP_SDK environment variable")
endif

#CC = clang 
$(info CC = $(CC))

# Compiler debug level
CFLAGS += -O3
#CFLAGS +=  -g -rdynamic -O0
#CFLAGS +=  -ggdb

#Turn warnings into errors
#CFLAGS += -Werror 
CFLAGS += -Wall 
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-unused-value
CFLAGS += -std=gnu99

ifeq ($(CC),gcc)
CFLAGS += -Wmaybe-uninitialized 
else
CFLAGS += -Wuninitialized
endif

#To define the backend used: odp/dpdk
CFLAGS += -D ODP_BK

CFLAGS += -D NDEBUG
CFLAGS += -D NINFO
CFLAGS += -D NSIGG

#CFLAGS += -fno-stack-protector
