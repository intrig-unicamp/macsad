#include "backend.h"
#include "dataplane.h"
#include "odp_tables.h"
#include "odp_api.h"
#include "stdio.h"
#include "odph_list_internal.h"
#ifndef debug
#define debug 1
#endif

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

/* TODO to be removed later */
/** @inner element structure of hash table                                       
 *  * To resolve the hash confict:                                                  
 *   * we put the elements with different keys but a same HASH-value                 
 *    * into a list                                                                   
 *     */                                                                              
typedef struct odph_hash_node {                                                  
	/** list structure,for list opt */                                           
	odph_list_object list_node;                                                  
	/** Flexible Array,memory will be alloced when table has been
	 * created        
	 *      * Its length is key_size + value_size,                                      
	 *           * suppose key_size = m; value_size = n;                                     
	 *                * its structure is like:                                                    
	 *                     * k_byte1 k_byte2...k_byten v_byte1...v_bytem                               
	 *                          */                                                                          
	char content[0];                                                             
} odph_hash_node;
typedef struct {                                                                 
	uint32_t magicword; /**< for check */                                        
	uint32_t key_size; /**< input param when create,in Bytes */                  
	uint32_t value_size; /**< input param when create,in Bytes */                
	uint32_t init_cap; /**< input param when create,in Bytes */                  
	/** multi-process support,every list has one rw lock */                      
	odp_rwlock_t *lock_pool;                                                     
	/** table bucket pool,every hash value has one list
	 * head */                  
	odph_list_head *list_head_pool;                                              
	/** number of the list head in list_head_pool */                             
	uint32_t head_num;                                                           
	/** table element pool */                                                    
	odph_hash_node *hash_node_pool;                                              
	/** number of element in the
	 * hash_node_pool */                               
	uint32_t hash_node_num;                                                      
	char rsv[7]; /**< Reserved,for alignment */                                  
	char name[ODPH_TABLE_NAME_LEN]; /**< table name */                           
} odph_hash_table_imp;                                                           


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


// ============================================================================
// HIGHER LEVEL TABLE MANAGEMENT

// ----------------------------------------------------------------------------
// CREATE

static void create_ext_table(lookup_table_t* t, void* table, int socketid)
{
	extended_table_t* ext = NULL;

#if 0
	odp_shm_t shm;

	/* Reserve memory for args from shared mem */

	if ((shm =odp_shm_lookup("ext_table")) != NULL) {
             odp_shm_free(shm);
	}

	shm = odp_shm_reserve("ext_table", sizeof(extended_table_t),
			ODP_CACHE_LINE_SIZE, 0);
	ext = odp_shm_addr(shm);
	if (ext == NULL) {
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
		exit(EXIT_FAILURE);
	}
#endif
	ext = malloc(sizeof(extended_table_t));
	memset(ext, 0, sizeof(extended_table_t));
	ext->odp_table = table;
	ext->size = 0;
	ext->content = malloc(sizeof(uint8_t*)*TABLE_MAX);
	memset(ext->content, 0, sizeof(uint8_t*)*TABLE_MAX);

	t->table = ext;
}

void table_create(lookup_table_t* t, int socketid, int replica_id)
{
	char name[MACS_TABLE_NAME_LEN];
	t->socketid = socketid;
	odph_table_t tbl;
	odph_table_ops_t *test_ops;
	printf(":::: EXECUTING table create:\n");
	switch(t->type) {
		case LOOKUP_EXACT:
			test_ops = &odph_hash_table_ops;
			snprintf(name, sizeof(name), "%s_exact_%d_%d", t->name, socketid, replica_id);
			if ((tbl = test_ops->f_lookup(name)) != NULL){
				printf("  ::table %s already present \n", name);
				test_ops->f_des(tbl);
			}
// name, capacity, key_size, value size
			tbl = test_ops->f_create(name, 2, t->key_size, t->val_size);
			if(tbl == NULL) {
				printf("  ::Table %s creation fail\n", name);
			    exit(0);
			}

            create_ext_table(t, tbl, socketid);
            break;
    }
	odph_hash_table_imp *tbl_tmp = (odph_hash_table_imp *)tbl;
	extended_table_t * ext = t->table;
	printf("  ::Table odp %p %s, lval_size %d created \n", tbl_tmp, tbl_tmp->name, tbl_tmp->value_size);
//printf("  ::Table lookup %p %s,type %d, lval_size %d, socket %d\n", t, t->name, t->type,t->val_size, socketid);
//printf(" ::lookup %p, ext %p, tbl %p \n", t, t->table, ext->odp_table);
}

// ----------------------------------------------------------------------------
// Set action value for tables

void table_setdefault(lookup_table_t* t, uint8_t* value)
{
   printf(":::: EXECUTING table_setdefault - val size %d, socket id %d\n", t->val_size, t->socketid);
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
	printf(":::: EXECUTING exact add on table %s \n", t->name);
	printf("  :: key:  %x:%x:%x:%x:%x:%x \n",key[0],key[1],key[2],key[3],key[4],key[5]);
	ret = test_ops->f_put(ext->odp_table, key, value);
	if (ret != 0) {
		printf("  ::EXACT table add key failed \n");
		exit(EXIT_FAILURE);
	}

#if 0 //test code to verify table entry success
	//	ret = test_ops->f_put(ext->odp_table, key, value);
//	char buffer[5];
	char *buffer = NULL;
	buffer = malloc(sizeof(char)*t->val_size);
	memset(buffer, 0, t->val_size);
	odph_hash_table_imp *tbl = (odph_hash_table_imp *)ext->odp_table;
	ret = test_ops->f_get(ext->odp_table, key, buffer, t->val_size);
	if (ret != 0) {
		printf(" ::  EXACT lookup failed after add \n");
		printf("   :  odp tbl name %s, value_size %d\n", tbl->name, tbl->value_size);
		printf("   :  lookup %p tbl name %s, value_size %d \n", t, t->name, t->val_size);
	}
	else {
		printf("  :: EXACT lookup Passed after add \n");
		printf("    :  odp tbl name %s, value_size %d\n", tbl->name, tbl->value_size);
		printf("   :  lookup %p tbl name %s, value_size %d \n", t, t->name, t->val_size);
	}
	free(buffer);
#endif
#if 0
	ext->content[index%256] = copy_to_socket(value, t->val_size, t->socketid);
#endif
}

void lpm_add(lookup_table_t* t, uint8_t* key, uint8_t depth, uint8_t* value)
{
	return;
}

void
ternary_add(lookup_table_t* t, uint8_t* key, uint8_t* mask, uint8_t* value)
{
	return;
}

// ----------------------------------------------------------------------------
// LOOKUP

uint8_t* exact_lookup(lookup_table_t* t, uint8_t* key)
{
	int ret = 0;
	void *buffer = NULL;
	// TODO need to free the memory somewhere ??
	buffer = malloc(sizeof(char)*t->val_size);
	memset(buffer, 0, t->val_size);
	odph_table_ops_t *test_ops;
	test_ops = &odph_hash_table_ops;
	extended_table_t* ext = (extended_table_t*)t->table;
#if debug == 1
	printf(":::: EXECUTING exact lookup on table %s \n", t->name);
	printf("  :: key:  %x:%x:%x:%x:%x:%x \n",key[0],key[1],key[2],key[3],key[4],key[5]);
#endif
	ret = test_ops->f_get(ext->odp_table, key, buffer, t->val_size);
	if (ret != 0) {
		printf("  :: EXACT lookup fail \n");
		return t->default_val;
	}
	printf("  :: EXACT lookup success \n");
	return buffer;
}

uint8_t* lpm_lookup(lookup_table_t* t, uint8_t* key)
{
    return 0;
}

uint8_t* ternary_lookup(lookup_table_t* t, uint8_t* key)
{
    return 0;
}

//---------
//DELETE
//TODO need to implement cleanup
void table_des (lookup_table_t* t){
//	clean up shm reserve mem
//	destroy odph table
	return;
}

void shm_release ()
{
	return;
}
