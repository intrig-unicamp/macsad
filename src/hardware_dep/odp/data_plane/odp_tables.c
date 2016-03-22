#include "backend.h"
#include "dataplane.h"
#include "odp_tables.h"
#include "odp_api.h"
#include "stdio.h"
//#include <helper/odph_hashtable.h>
// ============================================================================
// LOOKUP TABLE IMPLEMENTATIONS

#include "ternary_naive.h"  // TERNARY

#if 0
static uint8_t*
copy_to_socket(uint8_t* src, int length, int socketid) {
    uint8_t* dst = rte_malloc_socket("uint8_t", sizeof(uint8_t)*length, 0, socketid);
    memcpy(dst, src, length);
    return dst;
}
#endif
static uint8_t*
copy_to_socket(uint8_t* src, int length, int socketid) {
    return 0;
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

#if 0
int32_t
hash_add_key(struct rte_hash* h, void *key)
{
    int32_t ret;
    ret = rte_hash_add_key(h,(void *) key);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Unable to add entry to the hash.\n");
    return ret;
}
#endif

#if 0
void
lpm4_add(struct rte_lpm* l, uint32_t key, uint8_t depth, uint8_t value)
{
    int ret;
    ret = rte_lpm_add(l, key, depth, value);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Unable to add entry to the LPM table\n");
    printf("LPM: Added 0x%08x / %d (%d)\n", (unsigned)key, depth, value);
}
#endif

#if 0
void
lpm6_add(struct rte_lpm6* l, uint8_t key[16], uint8_t depth, uint8_t value)
{
    int ret;
    ret = rte_lpm6_add(l, key, depth, value);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Unable to add entry to the LPM table\n");
    printf("LPM: Adding route %s / %d (%d)\n", "IPV6", depth, value);
}
#endif

// ============================================================================
// HIGHER LEVEL TABLE MANAGEMENT

// ----------------------------------------------------------------------------
// CREATE

static void
create_ext_table(lookup_table_t* t, void* table, int socketid)
{
	extended_table_t* ext = NULL;
        odp_shm_t shm;
	/* Reserve memory for args from shared mem */
	shm = odp_shm_reserve("ext_table", sizeof(extended_table_t),
			ODP_CACHE_LINE_SIZE, 0);
	ext = odp_shm_addr(shm);
	if (ext == NULL) {
//		EXAMPLE_ERR("Error: shared mem alloc failed.\n");
		exit(EXIT_FAILURE);
	}
	memset(ext, 0, sizeof(*ext));

//    extended_table_t* ext = rte_malloc_socket("extended_table_t", sizeof(extended_table_t), 0, socketid);
//    ext->content = rte_malloc_socket("uint8_t*", sizeof(uint8_t*)*TABLE_MAX, 0, socketid);
	ext->odp_table = table;
	ext->size = 0;

	/* Reserve memory for args from shared mem */
	shm = odp_shm_reserve("ext_table_content", sizeof(uint8_t*)*TABLE_MAX,
			ODP_CACHE_LINE_SIZE, 0);
	ext->content = odp_shm_addr(shm);
	if (ext == NULL) {
		//EXAMPLE_ERR("Error: shared mem alloc failed.\n");
		exit(EXIT_FAILURE);
	}
	memset(ext->content, 0, sizeof(*(ext->content)));

	t->table = ext;
}

void
table_create(lookup_table_t* t, int socketid)
{
    char name[64];
    t->socketid = socketid;
    odph_table_t table;
    odph_table_ops_t *test_ops;

	printf("test hash table:\n");
        test_ops = &odph_hash_table_ops;

    switch(t->type)
    {
        case LOOKUP_EXACT:
            snprintf(name, sizeof(name), "%s_exact_%d", t->name, socketid);
//            table = odph_table_create(name, TABLE_SIZE, t->key_size, TABLE_VALUE_SIZE);
            //*table = test_ops->f_create(name, TABLE_SIZE, t->key_size, TABLE_VALUE_SIZE);
            table = test_ops->f_create("test", 2, 4, 16);
	if(table == NULL) {
	printf("table create fail\n");
exit(0);
}

            //struct rte_hash* h = hash_create(socketid, name, t->key_size, crc32);
            create_ext_table(t, table, socketid);
            break;
#if 0
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
#endif
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
#if 0
    extended_table_t* ext = (extended_table_t*)t->table;
    uint32_t index = rte_hash_add_key(ext->rte_table, (void*) key);
    if(index < 0)
        rte_exit(EXIT_FAILURE, "HASH: add failed\n");
    ext->content[index%256] = copy_to_socket(value, t->val_size, t->socketid);
#endif
}

void
lpm_add(lookup_table_t* t, uint8_t* key, uint8_t depth, uint8_t* value)
{
#if 0
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
#endif
}

void
ternary_add(lookup_table_t* t, uint8_t* key, uint8_t* mask, uint8_t* value)
{
#if 0
    naive_ternary_add(t->table, key, mask, copy_to_socket(value, t->val_size, t->socketid));
#endif
}

// ----------------------------------------------------------------------------
// LOOKUP

uint8_t*
exact_lookup(lookup_table_t* t, uint8_t* key)
{
#if 0
    printf("lookup %p\n", t);
    int ret = 0;
    ret = rte_hash_lookup(ext->rte_table, key);
    return (ret < 0)? t->default_val : ext->content[ret % 256];
#endif
//return any int, remove later
    return 0;
}

uint8_t*
lpm_lookup(lookup_table_t* t, uint8_t* key)
{
    return NULL;
}

uint8_t*
ternary_lookup(lookup_table_t* t, uint8_t* key)
{
#if 0
    uint8_t* ret = naive_ternary_lookup(t->table, key);
    return ret == NULL ? t->default_val : ret;
#endif
//return int, remove later
    return 0;
}
