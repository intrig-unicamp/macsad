#include "backend.h"
#include "dataplane.h"
#include "odp_tables.h"
#include "odp_api.h"
#include "stdio.h"
#include "odph_list_internal.h"
#include "odp_lib.h"

#include "actions.h"
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


static void print_prefix_info(
        const char *msg, uint32_t ip, uint8_t cidr)
{
    int i = 0;
    uint8_t *ptr = (uint8_t *)(&ip);

    printf("%s IP prefix: ", msg);
    for (i = 3; i >= 0; i--) {
        if (i != 3)
            printf(".");
        printf("%d", ptr[i]);
    }
    printf("/%d\n", cidr);
}

// ============================================================================
// SIMPLE HASH FUNCTION FOR EXACT TABLES

static uint32_t crc32(const void *data, uint32_t data_len, uint32_t init_val) {
    int32_t *data32 = (void*)data;
    uint32_t result = init_val;
    result = _mm_crc32_u32 (result, *data32++);
    return result;
}

#if 0
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
#endif

// ----------------------------------------------------------------------------
// CREATE

static void create_ext_table(lookup_table_t* t, void* table, int socketid)
{
    extended_table_t* ext = NULL;

    ext = malloc(sizeof(extended_table_t));
    memset(ext, 0, sizeof(extended_table_t));
    ext->odp_table = table;
    ext->size = 0;
    ext->content = malloc(sizeof(uint8_t*)*TABLE_SIZE);
    memset(ext->content, 0, sizeof(uint8_t*)*TABLE_SIZE);

    t->table = ext;
}

void table_create(lookup_table_t* t, int socketid, int replica_id)
{
    char name[MACS_TABLE_NAME_LEN];
    t->socketid = socketid;
    odph_table_t tbl;
    if(t->key_size == 0) return; // we don't create the table if there are no keys (it's a fake table for an element in the pipeline)
    info(":::: EXECUTING table create:\n");
    switch(t->type) {
        case LOOKUP_EXACT:
            snprintf(name, sizeof(name), "%s_exact_%d_%d", t->name, socketid, replica_id);
#ifndef CUCKOO
            if ((tbl = odph_hash_table_lookup(name)) != NULL){
                info("  ::table %s already present \n", name);
                odph_hash_table_destroy(tbl);
            }
            // name, capacity, key_size, value size
            tbl = odph_hash_table_create(name, TABLE_SIZE, t->key_size, TABLE_VALUE_SIZE);
#else
            if ((tbl = odph_cuckoo_table_lookup(name)) != NULL){
                info("  ::table %s already present \n", name);
                odph_cuckoo_table_destroy(tbl);
            }
            // name, capacity, key_size, value size
            tbl = odph_cuckoo_table_create(name, TABLE_SIZE, t->key_size, TABLE_VALUE_SIZE);
#endif
            if(tbl == NULL) {
                debug("  ::Table %s creation fail\n", name);
                debug("  ::Table size %d, key size %d, val_size %d\n", TABLE_SIZE, t->key_size, TABLE_VALUE_SIZE);
                exit(0);
            }

            create_ext_table(t, tbl, socketid);
            //debug("  ::Table %s creation complete\n", name);
            debug("  ::Table %s, Table size %d, key size %d, val_size %d created \n", name, t->key_size, TABLE_SIZE, TABLE_VALUE_SIZE);
            break;
        case LOOKUP_LPM:
            snprintf(name, sizeof(name), "%s_lpm_%d_%d", t->name, socketid, replica_id);
            if ((tbl = odph_iplookup_table_lookup(name)) != NULL){
                info("  ::table %s already present \n", name);
                odph_iplookup_table_destroy(tbl);
            }
            // name, capacity, key_size, value size
            tbl = odph_iplookup_table_create(name, 2, t->key_size, t->val_size);
            if(tbl == NULL) {
                debug("  ::Table %s creation fail\n", name);
                exit(0);
            }

            create_ext_table(t, tbl, socketid);
            debug("  ::Table %s, key size %d, val_size %d created \n", name,t->key_size, t->val_size);
#if 0
                info("  ::table %s is created and verified \n", name);
            }
#endif
            break;
    }
}

static uint8_t* add_index(uint8_t* value, int val_size, int index)
{
    //	realloc doesn't work in this case ("invalid old size")
    uint8_t* value2 = malloc(val_size+sizeof(int));
    memcpy(value2, value, val_size);
    *(int*)(value2+val_size) = index;
    return value2;
}

void table_setdefault(lookup_table_t* t, uint8_t* value)
{
    info(":::: EXECUTING table_setdefault - val size %d, socket id %d\n", t->val_size, t->socketid);
    debug("Default value set for table %s (on socket %d).\n", t->name, t->socketid);
    value = add_index(value, t->val_size, DEFAULT_ACTION_INDEX);
    if(t->default_val) free(t->default_val);
    t->default_val = copy_to_socket(value, t->val_size+sizeof(int), t->socketid);

}

void exact_add(lookup_table_t* t, uint8_t* key, uint8_t* value)
{
    int ret = 0;
    extended_table_t* ext = (extended_table_t*)t->table;
    if(t->key_size == 0) return; // don't add lines to keyless tables
    info(":::: EXECUTING exact add on table %s, keysize %d, val size %d \n", t->name,t->key_size, t->val_size);
    value = add_index(value, t->val_size, t->counter++);
    ext->content[ext->size] = copy_to_socket(value, t->val_size+sizeof(int), t->socketid);
#ifndef CUCKOO
    ret = odph_hash_put_value(ext->odp_table, key, &(ext->size));
#else
    ret = odph_cuckoo_table_put_value(ext->odp_table, key, &(ext->size));
#endif
    ext->size++;
    if (ret < 0) {
        debug("  ::EXACT table add key failed \n");
        exit(EXIT_FAILURE);
    }
    info(":::: exact add success on table %s, entryIndex %d \n", t->name,ext->size);

    #if 0
    int result;
    printf("size of result %d int %d\n", sizeof(result), sizeof(int));
    printf("Table %p content %p \n",ext, ext->content);
    ret = odph_cuckoo_table_get_value(ext->odp_table, key, &result, sizeof(result));
    printf("Table %p content %p \n",ext, ext->content);
    //ext = (extended_table_t*)t->table;
    //printf("o2 Table %p content %p \n",ext, ext->content);
    if (ret < 0) {
        debug("  :: EXACT lookup aft4 add fail with ret=%d,result=%d \n", ret, result);
    }
    info(":::: exact lookup success:Table %s, entryIndex %d \n", t->name,ext->size);
    value = ext->content[result];
    printf("Value: %02x\n", (unsigned char) value[0]);
    if(NULL != ext->content[result]){
        value = ext->content[result];
    printf("ADD-Lookup Value: %02x:%02x:%02x:%02x\n",
                (unsigned char) value[0],
                (unsigned char) value[1],
                (unsigned char) value[3],
                (unsigned char) value[4]);
    }
#endif
}

void lpm_add(lookup_table_t* t, uint8_t* key, uint8_t depth, uint8_t* value)
{
    int ret = 0;
    extended_table_t* ext = (extended_table_t*)t->table;

    if(t->key_size == 0) return; // don't add lines to keyless tables

    key[4] = depth; //adding depth to key[4]
    uint8_t* value2=NULL;
    //uint8_t* value2 = value;

    odph_iplookup_prefix_t prefix;
    for (int i = 0; i < ODPH_IPV4ADDR_LEN; i++)
        if (key[i] > 255)
            return; //TODO how to handle return here

    prefix.ip = key[0] << 24 | key[1] << 16 | key[2] << 8 | key[3];
    prefix.cidr = key[4];
    //prefix.cidr = 16;

    info(":::: EXECUTING lpm add on table %s, depth %d, keysize %d valsize %d, value %p \n", t->name, depth, t->key_size, t->val_size, value);
    info("  :: key:  %d:%d:%d:%d - %d \n",key[0],key[1],key[2],key[3],key[4]);

    value = add_index(value, t->val_size, t->counter++);
    ext->content[ext->size] = copy_to_socket(value, t->val_size+sizeof(int), t->socketid);
    value2 = malloc(t->val_size);
    memcpy(value2, value, t->val_size);
    printf("addr value2 %p,value2 %p \n", &value2,value2);

    print_prefix_info("Add", prefix.ip, prefix.cidr);
    ret = odph_iplookup_table_put_value(ext->odp_table, &prefix, &value2);
    ext->size++;
    if (ret == -1) {
        debug("  ::LPM table %s add key failed for indexID=%d\n", t->name, ext->size);
        exit(EXIT_FAILURE);
    }
    else {
        debug("  ::LPM table %s add key success for IndexID=%d\n", t->name,ext->size);
    }
/*
    uint8_t* result = NULL;
 printf("B4-Addr result %p,result %p \n", &result,result);
    ret = odph_iplookup_table_get_value(ext->odp_table, &(prefix.ip), &result, t->val_size);
 printf("addr value2 %p,value2 %p \n", &value2,value2);
 printf("After-Addr result %p,result %p \n", &result,result);
    if (ret < 0) {
        printf("Failed to find longest prefix with result %p \n", &result);
        debug("  :: LPM lookup fail \n");
    }
    info("  :: LPM lookup success with result=%d\n", result);
*/
    return;
}

void ternary_add(lookup_table_t* t, uint8_t* key, uint8_t* mask, uint8_t* value)
{
    return;
}

// LOOKUP

uint8_t* exact_lookup(lookup_table_t* t, uint8_t* key)
{
    int ret = 0;
    int result = 0;
    if(t->key_size == 0) return t->default_val;
    extended_table_t* ext = (extended_table_t*)t->table;
    info(":::: EXECUTING exact lookup on table %s, keysize %d \n", t->name,t->key_size);
    info ("::: exact_lookup -key- %p,key0- %d,key1- %d \n", key, key[0], key[1]);
#ifndef CUCKOO
    ret = odph_hash_get_value(ext->odp_table, key, &result, TABLE_VALUE_SIZE);
#else
     ret = odph_cuckoo_table_get_value(ext->odp_table, key, &result, TABLE_VALUE_SIZE);
#endif
    if (ret < 0) {
        debug("  :: EXACT lookup fail with ret=%d,result=%d \n", ret, result);
        return t->default_val;
    }

    info("  :: EXACT lookup success with result=%d \n",result);
    if (NULL == ext->content[result])
    {
        printf("ERROR: ResultID: %d, Value is Null \n",result);
        return t->default_val;
    }else {
        return ext->content[result];
    }
}

uint8_t* lpm_lookup(lookup_table_t* t, uint8_t* key)
{
    int ret = 0;
    uint8_t *result = NULL;
    uint32_t lkp_ip = 0;
    if(t->key_size == 0) return t->default_val;
    extended_table_t* ext = (extended_table_t*)t->table;
    info(":::: EXECUTING lpm lookup on table %s, keysize %d \n", t->name, t->key_size);
    debug("  :: key:  %d:%d:%d:%d - %d \n",key[0],key[1],key[2],key[3],key[4]);

    //odph_iplookup_prefix_t lkp_ip;
    for (int i = 0; i < ODPH_IPV4ADDR_LEN; i++)
        if (key[i] > 255)
            return NULL; //TODO how to handle return here

    lkp_ip = key[0] << 24 | key[1] << 16 | key[2] << 8 | key[3];
    //  lkp_ip.cidr = key[4];
#ifndef NINFO
    print_prefix_info("Lookup", lkp_ip, key[4]);
    //print_prefix_info("Lookup", lkp_ip, lkp_ip.cidr);
#endif
    ret = odph_iplookup_table_get_value(ext->odp_table, &lkp_ip, &result, t->val_size);
    if (ret < 0) {
        printf("Failed to find longest prefix with result %p \n", &result);
        debug("  :: LPM lookup fail \n");
        return t->default_val;
    }
    info("  :: LPM lookup success with result=%d\n", (int)*result);

    /*
       print_prefix_info("Lkp", lkp_ip, 32);
       if (ret < 0 || result != 1) {
       printf("Error: found result ater deleting\n");
       if (ret < 0 || result != 2) {
       printf("Failed to find longest prefix\n");
       */
    //return ext->content[result];
    return result;
}

uint8_t* ternary_lookup(lookup_table_t* t, uint8_t* key)
{
    return 0;
}

//---------
//DELETE
//TODO need to implement cleanup
void odpc_tbl_des (lookup_table_t* t){
    //	Use destroy odph table apis
    return;
}

// ============================================================================
// HIGHER LEVEL TABLE MANAGEMENT

/*
   Create table for each socket (CPU).
   Create replica set of tables too.
   */
static void create_tables_on_socket (int socketid)
{
    //only if the table is defined in p4 prog
    //if (table_config == NULL) return; //always present
    int i;
    for (i=0;i < NB_TABLES; i++) {
        debug("creting table with tableID  %d \n", i);
        lookup_table_t t = table_config[i];
        int j;
        for(j = 0; j < NB_REPLICA; j++) {
            state[socketid].tables[i][j] = malloc(sizeof(lookup_table_t));
            memcpy(state[socketid].tables[i][j], &t, sizeof(lookup_table_t));
            table_create(state[socketid].tables[i][j], socketid, j);
        }
        state[socketid].active_replica[i] = 0;
    }
}

/*
   Initialize the lookup tables for dataplane.
   TODO make it void
   */
int odpc_lookup_tbls_init()
{
    int socketid = SOCKET_DEF;
    unsigned lcore_id;
    info("Initializing tables...\n");
    for (lcore_id = 0; lcore_id < MAC_MAX_LCORE; lcore_id++) {
        /*
           if (rte_lcore_is_enabled(lcore_id) == 0) continue;
           if (numa_on) socketid = rte_lcore_to_socket_id(lcore_id);
           else socketid = 0;
           if (socketid >= NB_SOCKETS) {
           rte_exit(EXIT_FAILURE, "Socket %d of lcore %u is out of range %d\n",
           socketid, lcore_id, NB_SOCKETS);
           }
           */
        if (state[socketid].tables[0][0] == NULL) {
            create_tables_on_socket(socketid);
            //            create_counters_on_socket(socketid);
        }
    }
    //create_registers();
    info("Initializing tables Done.\n");
    return 0;
}

/*
   Initialize the lookup tables for dataplane.
   TODO make it void
   */
int odpc_lookup_tbls_des()
{
    int socketid = SOCKET_DEF;
    int i, j;
    unsigned lcore_id;
    info("Destroying Lookup tables...\n");
    for (lcore_id = 0; lcore_id < MAC_MAX_LCORE; lcore_id++) {
        for(i = 0; i < NB_TABLES; i++) {
            for(j = 0; j < NB_REPLICA; j++) {
                if (state[socketid].tables[i][j] != NULL){
                    odpc_tbl_des (state[socketid].tables[0][0]);
                }
            }
        }
    }
    /* Success */
    info("Destroying Lookup tables done.\n");
    return 0;
}
