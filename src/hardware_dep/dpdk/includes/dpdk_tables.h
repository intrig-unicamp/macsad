#ifndef DPDK_TABLES_H
#define DPDK_TABLES_H

typedef struct extended_table_s {
    void*     rte_table;
    uint8_t   size;
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

#endif
