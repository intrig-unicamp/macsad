#include "backend.h"
#include "dataplane.h"
#include "dpdk_tables.h"

// ============================================================================
// LOOKUP TABLE IMPLEMENTATIONS

#include <rte_hash.h>       // EXACT
#include <rte_lpm.h>        // LPM (32 bit key)
#include <rte_lpm6.h>       // LPM (128 bit key)
#include "ternary_naive.h"  // TERNARY

#include <rte_malloc.h>     // extended tables
#include <rte_version.h>    // for conditional on rte_hash parameters

static uint8_t*
copy_to_socket(uint8_t* src, int length, int socketid) {
    uint8_t* dst = rte_malloc_socket("uint8_t", sizeof(uint8_t)*length, 0, socketid);
    memcpy(dst, src, length);
    return dst;
}

// ============================================================================
// SIMPLE HASH FUNCTION FOR EXACT TABLES

static uint32_t crc32(const void *data, uint32_t data_len,	uint32_t init_val) {
    const uint8_t* bytes = data;
    uint32_t crc, mask;
    int i, j;
    i = 0;
    crc = 0xFFFFFFFF;
    while (i < data_len) {
        crc = crc ^ bytes[i];
        for (j = 7; j >= 0; j--) {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
        i = i + 1;
    }
    return ~crc;
}

// ============================================================================
// LOWER LEVEL TABLE MANAGEMENT

struct rte_hash *
hash_create(int socketid, const char* name, uint32_t keylen, rte_hash_function hashfunc)
{
    struct rte_hash_parameters hash_params = {
        .name = NULL,
        .entries = HASH_ENTRIES,
#if RTE_VER_MAJOR == 2 && RTE_VER_MINOR == 0
        .bucket_entries = 4,
#endif
        .key_len = keylen,
        .hash_func = hashfunc,
        .hash_func_init_val = 0,
    };
    hash_params.name = name;
    hash_params.socket_id = socketid;
    struct rte_hash *h = rte_hash_create(&hash_params);
    if (h == NULL)
        rte_exit(EXIT_FAILURE, "DPDK: Unable to create the hash on socket %d\n", socketid);
    return h;
}

struct rte_lpm *
lpm4_create(int socketid, const char* name, uint8_t max_size)
{
    struct rte_lpm *l = rte_lpm_create(name, socketid, max_size, 0/*flags*/);
    if (l == NULL)
        rte_exit(EXIT_FAILURE, "DPDK: Unable to create the LPM table on socket %d\n", socketid);
    return l;
}

struct rte_lpm6 *
lpm6_create(int socketid, const char* name, uint8_t max_size)
{
    struct rte_lpm6_config config;
    config.max_rules = max_size;
    //config.number_tbl8s = LPM6_NUMBER_TBL8S;
    config.flags = 0;
    struct rte_lpm6 *l = rte_lpm6_create(name, socketid, &config);
    if (l == NULL)
        rte_exit(EXIT_FAILURE, "DPDK: Unable to create the LPM6 table on socket %d\n", socketid);
    return l;
}

int32_t
hash_add_key(struct rte_hash* h, void *key)
{
    int32_t ret;
    ret = rte_hash_add_key(h,(void *) key);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Unable to add entry to the hash.\n");
    return ret;
}

void
lpm4_add(struct rte_lpm* l, uint32_t key, uint8_t depth, uint8_t value)
{
    int ret;
    ret = rte_lpm_add(l, key, depth, value);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Unable to add entry to the LPM table\n");
    printf("LPM: Added 0x%08x / %d (%d)\n", (unsigned)key, depth, value);
}

void
lpm6_add(struct rte_lpm6* l, uint8_t key[16], uint8_t depth, uint8_t value)
{
    int ret;
    ret = rte_lpm6_add(l, key, depth, value);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Unable to add entry to the LPM table\n");
    printf("LPM: Adding route %s / %d (%d)\n", "IPV6", depth, value);
}

// ============================================================================
// HIGHER LEVEL TABLE MANAGEMENT

// ----------------------------------------------------------------------------
// CREATE

static void
create_ext_table(lookup_table_t* t, void* rte_table, int socketid)
{
    extended_table_t* ext = rte_malloc_socket("extended_table_t", sizeof(extended_table_t), 0, socketid);
    ext->rte_table = rte_table;
    ext->size = 0;
    ext->content = rte_malloc_socket("uint8_t*", sizeof(uint8_t*)*TABLE_MAX, 0, socketid);
    t->table = ext;
}

void
table_create(lookup_table_t* t, int socketid)
{
    char name[64];
    t->socketid = socketid;
    switch(t->type)
    {
        case LOOKUP_EXACT:
            snprintf(name, sizeof(name), "%s_exact_%d", t->name, socketid);
            struct rte_hash* h = hash_create(socketid, name, t->key_size, crc32);
            create_ext_table(t, h, socketid);
            break;
        case LOOKUP_LPM:
            snprintf(name, sizeof(name), "%s_lpm_%d", t->name, socketid);
            if(t->key_size <= 4)
            {
                struct rte_lpm* l = lpm4_create(socketid, name, t->max_size);
                create_ext_table(t, l, socketid);
                break;
            }
            else if(t->key_size <= 16)
            {
                struct rte_lpm6* l = lpm6_create(socketid, name, t->max_size);
                create_ext_table(t, l, socketid);
                break;
            }
            else
                rte_exit(EXIT_FAILURE, "LPM: key_size not supported\n");
        case LOOKUP_TERNARY:
            t->table = naive_ternary_create(t->key_size, t->max_size);
            break;
    }
    printf("Created table of type %d on socket %d\n", t->type, socketid);
}

// ----------------------------------------------------------------------------
// SET DEFAULT VALUE

void
table_setdefault(lookup_table_t* t, uint8_t* value)
{
   printf("set_default - %d, %d, %p\n", t->val_size, t->socketid, t);
    t->default_val = copy_to_socket(value, t->val_size, t->socketid);
}

// ----------------------------------------------------------------------------
// ADD

void
exact_add(lookup_table_t* t, uint8_t* key, uint8_t* value)
{
    extended_table_t* ext = (extended_table_t*)t->table;
    uint32_t index = rte_hash_add_key(ext->rte_table, (void*) key);
    if(index < 0)
        rte_exit(EXIT_FAILURE, "HASH: add failed\n");
    ext->content[index%256] = copy_to_socket(value, t->val_size, t->socketid);
}

void
lpm_add(lookup_table_t* t, uint8_t* key, uint8_t depth, uint8_t* value)
{
    extended_table_t* ext = (extended_table_t*)t->table;
    if(t->key_size <= 4)
    {
        ext->content[ext->size] = copy_to_socket(value, t->val_size, t->socketid);
        // the rest is zeroed in case of keys smaller then 4 bytes
        uint32_t* key32 = calloc(1, sizeof(uint32_t));
        memcpy(key32, key, t->key_size);
        lpm4_add(ext->rte_table, *key32, depth, ext->size++);
    }
    else if(t->key_size <= 16)
    {
        ext->content[ext->size] = copy_to_socket(value, t->val_size, t->socketid);
        // the rest is zeroed in case of keys smaller then 16 bytes
        uint8_t* key128 = calloc(16, sizeof(uint8_t));
        memcpy(key128, key, t->key_size);
        lpm6_add(ext->rte_table, key128, depth, ext->size++);
    }
}

void
ternary_add(lookup_table_t* t, uint8_t* key, uint8_t* mask, uint8_t* value)
{
    naive_ternary_add(t->table, key, mask, copy_to_socket(value, t->val_size, t->socketid));
}

// ----------------------------------------------------------------------------
// LOOKUP

uint8_t*
exact_lookup(lookup_table_t* t, uint8_t* key)
{
    #if debug == 1
    printf("lookup %p\n", t);
    #endif
    extended_table_t* ext = (extended_table_t*)t->table;
    int ret = 0;
    ret = rte_hash_lookup(ext->rte_table, key);
    return (ret < 0)? t->default_val : ext->content[ret % 256];
}

uint8_t*
lpm_lookup(lookup_table_t* t, uint8_t* key)
{
    extended_table_t* ext = (extended_table_t*)t->table;
    if(t->key_size <= 4)
    {
        uint32_t* key32 = calloc(1, sizeof(uint32_t));
        memcpy(key32, key, t->key_size);
        int ret = 0;
        uint8_t result;
        ret = rte_lpm_lookup(ext->rte_table, *key32, &result);
        return ret == 0 ? ext->content[result] : t->default_val;
    }
    else if(t->key_size <= 16)
    {
        uint8_t* key128 = calloc(16, sizeof(uint8_t));
        memcpy(key128, key, t->key_size);
        int ret = 0;
        uint8_t result;
        ret = rte_lpm6_lookup(ext->rte_table, key128, &result);
        return ret == 0 ? ext->content[result] : t->default_val;
    }
    return NULL;
}

uint8_t*
ternary_lookup(lookup_table_t* t, uint8_t* key)
{
    uint8_t* ret = naive_ternary_lookup(t->table, key);
    return ret == NULL ? t->default_val : ret;
}
