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

ifeq ($(ODP_SDK),)
$(error "Please define ODP_SDK environment variable")
endif

#CC = clang
$(info CC = $(CC))

# Compiler debug level
CFLAGS += -O3
#CFLAGS +=  -g -O2
#CFLAGS +=  -g -rdynamic -O2
#CFLAGS += -fno-omit-frame-pointer
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

#Comment the flags to enable different level of debug
CFLAGS += -D NDEBUG
CFLAGS += -D NINFO
CFLAGS += -D NSIGG

#Comment to add hash table support instead of cuckoo table support
CFLAGS += -D CUCKOO

#CFLAGS += -fno-stack-protector
