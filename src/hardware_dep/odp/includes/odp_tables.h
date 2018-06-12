// Copyright 2018 INTRIG/FEEC/UNICAMP (University of Campinas), Brazil
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

#ifndef DPDK_TABLES_H
#define DPDK_TABLES_H

#include <odp/helper/table.h>
#include <odp/helper/odph_api.h>

typedef struct extended_table_s {
    void*     odp_table;
    uint32_t   size;
    uint8_t** content;
} extended_table_t;

//=============================================================================
// Table size limits

#ifdef RTE_ARCH_X86_64
/* default to 4 million hash entries (approx) */
#define HASH_ENTRIES		1024*1024*4
#else
/* 32-bit has less address-space for hugepage memory, limit to 1M entries */
#define HASH_ENTRIES		1024*1024*1
#endif
#define LPM_MAX_RULES         1024
#define LPM6_NUMBER_TBL8S (1 << 16)

#define TABLE_MAX 256

#define TABLE_SIZE 10000 //Number of elements table may store
//define different key and value size for different tables.
#define TABLE_KEY_SIZE 4 //key_size    fixed size of the 'key' in bytes.
#define TABLE_VALUE_SIZE 4 //value_size  fixed size of the 'value' in bytes.

int odpc_lookup_tbls_init();
int odpc_lookup_tbls_des();

#endif
