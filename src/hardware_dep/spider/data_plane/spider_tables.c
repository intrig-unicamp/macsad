#include "spider_tables.h"
#include "../../shared/data_plane/shared_tables.c"

#include "backend.h"
#include "dataplane.h"

#include "ternary_naive.h"  // TERNARY
#include "efs_fg.h"
#include "efs_fg.c"
#include "ich_itmh_fg.h"
#include "stddef.h"
#include "fabl_api.h"
#include "epp_rbtree.h"
#include "efs_lp.h"
#include "attr.h"
#include "pkthdr_utils_fg.h"
#include "modlog_epp.h"
#include "defines.h"
#include "ich_itmh_fg.h"
#include "ssc_rbos_fabl_types.h"
#include "spad_utils.h"
#include "efs.c"
#include "attr.h"
#include "ip_common.h"
#include "ip_route.h"
#include "fib/fib_table_common.h"
#include "fib/fib_table_attr.h"
#include "fib/fib_table.h"
#include "fib/fib_result.h"

#define FIB_ID_STRIDE_1_LENGTH  (5)
#define FIB_ID_STRIDE_2_LENGTH  (FIB_ID_SIZE - FIB_ID_STRIDE_1_LENGTH)

static void
ip_route_result_dereference (fib_reason_t reason UNUSED,
                             fib_table_type_t table_type UNUSED,
                             void *client_data)

{
    fib_result_control_t *result = (fib_result_control_t *) client_data;

    if (result) {
        fib_result_control_dereference(result);
    }
}

static fib_table_attr_t fib_table_ipv4_attr = {
   .table_type   = FIB_TABLE_TYPE_IPV4,
   .lookup_type  =  FIB_TABLE_LOOKUP_LPM,
   .turbo_strides = 3,
   .superfast_strides = 3,
   .max_strides  = 6,
   .stride_lengths = {FIB_ID_STRIDE_1_LENGTH, FIB_ID_STRIDE_2_LENGTH,
                      8, 12, 4, 8},
   .max_key_field = 2,
   .key_field = /* provision offset, lookup offset, field length */
                {{PROV_FIB_ID_START, LKUP_FIB_ID_START, FIB_ID_SIZE},
                 {PROV_IP_DST_ADDR_START, LKUP_IP_DST_ADDR_START,
                     FIB_IPV4_ADDR_KEYBIT_SIZE}},
   .leaf_size = FIB_SLE_LEAF_SIZE,
   .batch_ms   =  100,
   .batch_coalesce_count_max = 10,
   .batch_coalesce_threshold_messages = 3000,
   .deref_func = ip_route_result_dereference,
   .table_description = "Fib IPv4 sle table",
};

static fib_table_attr_t fib_table_ipv6_attr = {
   .table_type   = FIB_TABLE_TYPE_IPV6,
   .lookup_type  =  FIB_TABLE_LOOKUP_LPM,
   .turbo_strides = 4,
   .superfast_strides = 2,
   .max_strides  = 16,
   .stride_lengths = {FIB_ID_STRIDE_1_LENGTH, FIB_ID_STRIDE_2_LENGTH,
                      11,8,10,11,12,12, 8,8,8,8, 8,8,8,8},
   .max_key_field = 2,
   .key_field = /* provision offset, lookup offset, field length */
                {{PROV_FIB_ID_START, LKUP_FIB_ID_START, FIB_ID_SIZE},
                 {PROV_IP_DST_ADDR_START, LKUP_IPV6_DST_ADDR_START,
                     FIB_IPV6_ADDR_KEYBIT_SIZE}},
   .leaf_size = FIB_SLE_LEAF_SIZE,
   .batch_ms   =  100,
   .batch_coalesce_count_max = 10,
   .batch_coalesce_threshold_messages = 3000,
   .deref_func = ip_route_result_dereference,
   .table_description = "Fib IPv6 sle table",
};

void
table_create(lookup_table_t* t, int socketid)
{
    char name[64];

    switch(t->type)
    {
        case LOOKUP_EXACT:
            snprintf(name, sizeof(name), "%s_exact_%d", t->name, socketid);
            sle_hash_table_config_t config;
            config.hash_func_id = SLE_HASH_CRC32_IEEE_802_3;
            config.hash_size = 15;
            config.max_key_size = 50;
            config.data_size = 64;
            config.discrim_leaf = sle_rpc_64b_leaf_t_DISCRIM_elem_info;
            config.num_turbo_levels = 1;
            config.num_superfast_levels = 2;
            config.key_is_inline = TRUE;
            table_hash h = sle_hash_create_table(&config);
            create_ext_table(t, h);
            break;
        case LOOKUP_LPM:
            snprintf(name, sizeof(name), "%s_lpm_%d", t->name, socketid);
            fib_table_sle_config_t *sle_config;
            fib_table_sle_image_t *sle_image;
            if(t->key_size <= 32)
            {
                fib_table_create (socketid, EPP_OFA_TABLE_LPM_IPV4);
                struct rte_lpm* l = fib_table_get(socketid, EPP_OFA_TABLE_LPM_IPV4);
                fib_table_sle_config(sle_config, fib_table_ipv4_attr);
                fib_table_sle_image(sle_image,sle_config,socketid, fib_table_ipv4_attr);


                create_ext_table(t, l);
                break;
            }
            else if(t->key_size <= 128)
            {
                fib_table_create (socketid, EPP_OFA_TABLE_LPM_IPV6);
                struct rte_lpm* l = fib_table_get(socketid, EPP_OFA_TABLE_LPM_IPV6);
                fib_table_sle_config(sle_config, fib_table_ipv6_attr);
                fib_table_sle_image(sle_image,sle_config,socketid, fib_table_ipv6_attr);
                create_ext_table(t, l);
                break;
            }
            else
                break;//rte_exit(EXIT_FAILURE, "LPM: key_size not supported\n");
        case LOOKUP_TERNARY:
            break;
    }
    printf("Created table of type %d on socket %d\n", t->type, socketid);
}

// ----------------------------------------------------------------------------
// SET DEFAULT VALUE

void
table_setdefault(lookup_table_t* t, uint8_t* value)
{
    t->default_val = copy(value, t->val_size); // TODO should do the malloc on the right socketid
}

// ----------------------------------------------------------------------------
// ADD

void
exact_add(lookup_table_t* t, uint8_t* key, uint8_t* value)
{
    extended_table_t* ext = (extended_table_t*)t->table;

    sle_leaf_t elem_info;
    memcpy(elem_info, value, sizeof(sle_32b_leaf_t));
    sle_hash_insert_elem (t->table, key, t->key_size, elem_info);

}

void
lpm_add(/*KELL a socket_id*/ lookup_table_t* t, uint8_t* key, uint8_t depth, uint8_t* value)
{
    extended_table_t* ext = (extended_table_t*)t->table;
    fib_result_control_t *result_entity = (fib_result_control_t *)value;

    if(t->key_size <= 32)
    {
        fib_prefix_add_result (socket_id, EPP_OFA_TABLE_LPM_IPV4, key, depth, result_entity);

    }
    else if(t->key_size <= 128)
    {
        fib_prefix_add_result(socket_id, EPP_OFA_TABLE_LPM_IPV6, key, depth, result_entity);
    }
}


// ----------------------------------------------------------------------------
// LOOKUP

uint8_t*
exact_lookup(lookup_table_t* t, uint8_t* key)
{

    sle_hash_node_array_obj_t *leaf_arrays;
    sle_hash_elem_t *helem;
    sle_hash_bucket_t *bucket;
    uint32_t leaf_idx;
    result_t rc;
    rc = sle_hash_find_locate_elem(t->table, key, t->key_size, &leaf_arrays, &leaf_idx, &bucket, &helem, TRUE);
    return helem->elem_info;
}

uint8_t*
lpm_lookup(lookup_table_t* t, uint8_t* key)
{
    /*__spad*/ fib_sle_table_fwd_t *sp_sle_table = __ppa_allocs(sizeof(fib_sle_table_fwd_t), FIB_SLE_TABLE_FWD_ALIGN);
    fib_sle_table_fetch(sp_sle_table, t->table);
    fib_cached_table_lookup_begin(sp_sle_table, key, t->key_size);
    sle_lookup_result_t /*__spad*/ *sle_lookup_result;
    sle_table_lookup_get_result_32(sle_lookup_result);
    return sle_lookup_result->u.data_32;
}
