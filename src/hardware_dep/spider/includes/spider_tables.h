#ifndef SPIDER_TABLES_H
#define SPIDER_TABLES_H

typedef struct extended_table_s {
    void*     rte_table;
    uint8_t   size;
    uint8_t   key_size;
    uint8_t** content;
} extended_table_t;

//=============================================================================
// Table size limits

#endif
