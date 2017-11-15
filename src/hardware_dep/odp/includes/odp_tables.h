#ifndef DPDK_TABLES_H
#define DPDK_TABLES_H

#include <table.h>
#include <odph_hashtable.h>
#include <odph_iplookuptable.h>
#include <odph_cuckootable.h>

#if 0
#include <rte_version.h>    // for conditional compilation

#if RTE_VERSION >= RTE_VERSION_NUM(17,05,0,0)
typedef uint32_t table_index_t;
#else
typedef uint8_t table_index_t;
#endif
#endif
typedef uint32_t table_index_t;

typedef struct extended_table_s {
    void*     odp_table;
    table_index_t  size;
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

#define TABLE_SIZE 1000 //Number of elements table may store
//define different key and value size for different tables.
#define TABLE_KEY_SIZE 4 //key_size    fixed size of the 'key' in bytes.
#define TABLE_VALUE_SIZE 4 //value_size  fixed size of the 'value' in bytes.

int odpc_lookup_tbls_init();
int odpc_lookup_tbls_des();

#endif
