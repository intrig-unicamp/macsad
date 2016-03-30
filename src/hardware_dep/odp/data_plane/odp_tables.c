#include "backend.h"
#include "dataplane.h"
#include "odp_tables.h"
#include "odp_api.h"
#include "stdio.h"


#ifndef debug
#define debug 1
#endif

//#include <helper/odph_hashtable.h>
// ============================================================================
// LOOKUP TABLE IMPLEMENTATIONS

#include "ternary_naive.h"  // TERNARY

//TODO need to verify again. What is the functionality of it? How default_val is used?
static uint8_t*
copy_to_socket(uint8_t* src, int length, int socketid) {
//    uint8_t* dst = rte_malloc_socket("uint8_t", sizeof(uint8_t)*length, 0, socketid);
    uint8_t* dst =	malloc(sizeof(uint8_t)*length);
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

static void create_ext_table(lookup_table_t* t, void* table, int socketid)
{
	extended_table_t* ext = NULL;
    odp_shm_t shm;
	/* Reserve memory for args from shared mem */

	if ((shm =odp_shm_lookup("ext_table")) != NULL) {
             odp_shm_free(shm);
	}

	shm = odp_shm_reserve("ext_table", sizeof(extended_table_t),
			ODP_CACHE_LINE_SIZE, 0);
	ext = odp_shm_addr(shm);

//	ext = malloc();
	if (ext == NULL) {
//		EXAMPLE_ERR("Error: shared mem alloc failed.\n");
		exit(EXIT_FAILURE);
	}
	memset(ext, 0, sizeof(*ext));

	ext->odp_table = table;
	ext->size = 0;

	/* Reserve memory for args from shared mem */
	if ((shm = odp_shm_lookup("ext_table_content")) != NULL) {
             odp_shm_free(shm);
	}
	shm = odp_shm_reserve("ext_table_content", sizeof(uint8_t*)*TABLE_MAX,
			ODP_CACHE_LINE_SIZE, 0);
	ext->content = odp_shm_addr(shm);
	if (ext->content == NULL) {
		//EXAMPLE_ERR("Error: shared mem alloc failed.\n");
		exit(EXIT_FAILURE);
	}
	memset(ext->content, 0, sizeof(*(ext->content)));

	t->table = ext;
}

void table_create(lookup_table_t* t, int socketid)
{
	char name[64];
	t->socketid = socketid;
	odph_table_t tbl;
	odph_table_ops_t *test_ops;

	printf("test hash table:\n");
	switch(t->type)
	{
		case LOOKUP_EXACT:
			test_ops = &odph_hash_table_ops;
			snprintf(name, sizeof(name), "%s_exact_%d", t->name, socketid);
			if ((tbl = test_ops->f_lookup("test1")) != NULL){
				printf("table %s already present \n", name);
				test_ops->f_des(tbl);
			}
// name, capacity, key_size, value size
			tbl = test_ops->f_create("test1", 2, t->key_size, t->val_size);
			if(tbl == NULL) {
				printf("table %s create fail\n", name);
			    exit(0);
			}

            create_ext_table(t, tbl, socketid);
            break;
    }
    printf("Created %s table of type %d on socket %d\n", name, t->type, socketid);
}

// ----------------------------------------------------------------------------
// SET DEFAULT VALUE

void table_setdefault(lookup_table_t* t, uint8_t* value)
{
   printf("set_default - %d, %d, %p\n", t->val_size, t->socketid, t);
    t->default_val = copy_to_socket(value, t->val_size, t->socketid);
}

// ----------------------------------------------------------------------------
// ADD

void exact_add(lookup_table_t* t, uint8_t* key, uint8_t* value)
{
	int ret = 0;
	odph_table_ops_t *test_ops;
	test_ops = &odph_hash_table_ops;
    extended_table_t* ext = (extended_table_t*)t->table;
	ret = test_ops->f_put(ext->odp_table, key, value);
   	if (ret != 0) {
		printf("EXACT table add key failed \n");
		exit(EXIT_FAILURE);
	}
#if 0
    ext->content[index%256] = copy_to_socket(value, t->val_size, t->socketid);
#endif
}

void lpm_add(lookup_table_t* t, uint8_t* key, uint8_t depth, uint8_t* value)
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
	int ret = 0;
	void *buffer = NULL;
	odph_table_ops_t *test_ops;
	test_ops = &odph_hash_table_ops;
	extended_table_t* ext = (extended_table_t*)t->table;
#if debug == 1
	printf(":::: EXECUTING TABLE exact lookup\n");
	printf("lookup %p\n", t);
#endif
	ret = test_ops->f_get(ext->odp_table, key, buffer, t->val_size);
	if (ret != 0) {
		printf("EXACT table lookup failed \n");
		//		exit(EXIT_FAILURE);
		return t->default_val;
	}
	return buffer;
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
//TODO return int, remove later
    return 0;
}

//---------
//DELETE
void table_del ();
void shm_release ();

