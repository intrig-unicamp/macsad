#include "dpdk_lib.h"
#include <assert.h>

extern struct lcore_conf lcore_conf[RTE_MAX_LCORE];

enum actions {action_alma, action_korte};

struct action_alma_params { int i1; };
struct action_korte_params { int i1; int i2; int i3; };

struct table1_action {
    int action_id;
    union {
        struct action_alma_params alma_params;
        struct action_korte_params korte_params;
    };
};

lookup_table_t table_config[NB_TABLES] = {
    {
        .name = "table1",
        .type = LOOKUP_LPM,
        .key_size = 4,
        .val_size = sizeof(struct table1_action),
        .min_size = 0,
        .max_size = 255
    },{
        .name = "table2",
        .type = LOOKUP_EXACT,
        .key_size = 4,
        .val_size = 3,
        .min_size = 0,
        .max_size = 255
    },{
        .name = "table3",
        .type = LOOKUP_TERNARY,
        .key_size = 4,
        .val_size = 3,
        .min_size = 0,
        .max_size = 255
    }
};

/*

header_type whatever {
    fields {
        field1 : 16;
        field2 : 16;
    }
}

table table1 {
  reads {
    whatever.field1 : exact;
    whatever.field2 : lpm;
  }
}

table table2 {
  reads {
    whatever.field1 : exact;
    whatever.field2 : exact;
  }
}

table table3 {
  reads {
    whatever.field1 : exact;
    whatever.field2 : ternary;
  }
}

*/


extern void table_setdefault_promote  (int tableid, uint8_t* value);
extern void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value);
extern void lpm_add_promote    (int tableid, uint8_t* key, uint8_t depth, uint8_t* value);
extern void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value);

// ============================================================================
// CAN BE GENERATED FROM THE ABOVE
// ============================================================================

static void
table1_add(uint8_t field1[2], uint8_t field2[2], uint8_t field2_prefix_length, struct table1_action action)
{
    uint32_t key = (field1[0] << 24) + (field1[1] << 16) + (field2[0] << 8) + field2[1];
    uint8_t prefix_length = 16 + field2_prefix_length; // 16 is the bitwidth of field1
    lpm_add_promote(TABLE1, (uint8_t*)&key, prefix_length, &action);
}

static void
table2_add(uint8_t field1[2], uint8_t field2[2], uint8_t* value)
{
    int32_t key = (field1[0] << 24) + (field1[1] << 16) + (field2[0] << 8) + field2[1];
    exact_add_promote(TABLE2, (uint8_t*)&key, value);
}

static void
table3_add(uint8_t field1[2], uint8_t field2[2], uint8_t field2_mask[2], uint8_t* value)
{
    uint32_t key = (field1[0] << 24) + (field1[1] << 16) + (field2[0] << 8) + field2[1];
    uint32_t mask = (0xffff << 16) + (field2_mask[0] << 8) + field2_mask[1];
    ternary_add_promote(TABLE3, (uint8_t*)&key, (uint8_t*)&mask, value);
}

static uint8_t* table1_lookup(lookup_table_t** tables, uint8_t field1[2], uint8_t field2[2])
{
    uint32_t key = (field1[0] << 24) + (field1[1] << 16) + (field2[0] << 8) + field2[1];
    return lpm_lookup(tables[TABLE1], (uint8_t*)&key);
}

static uint8_t* table2_lookup(lookup_table_t** tables, uint8_t field1[2], uint8_t field2[2])
{
    int32_t key = (field1[0] << 24) + (field1[1] << 16) + (field2[0] << 8) + field2[1];
	return exact_lookup(tables[TABLE2], (uint8_t*)&key);
}

static uint8_t* table3_lookup(lookup_table_t** tables, uint8_t field1[2], uint8_t field2[2])
{
    uint32_t key = (field1[0] << 24) + (field1[1] << 16) + (field2[0] << 8) + field2[1];
    return ternary_lookup(tables[TABLE3], (uint8_t*)&key);
}


// ============================================================================
// TEST IT
// ============================================================================

uint8_t a1[2] = {0xff, 0   };
uint8_t a2[2] = {0   , 0xff};
uint8_t a3[2] = {0   , 0xf0};
uint8_t m1[2] = {0   , 0xff};

static void
fill_table1()
{
    struct table1_action value;
    value.action_id = action_korte;
    value.korte_params = (struct action_korte_params){11, 22, 33};
    table1_add(a1, a2, 8, value);
}

static void
fill_table2()
{
    uint8_t value[3] = {4,5,6};
    table2_add(a1, a2, &value);

    uint8_t value2[3] = {44,55,66};
    table_setdefault_promote(TABLE2, &value2);
}

static void
fill_table3()
{
    uint8_t value[3] = {7,8,9};
    table3_add(a1, a2, m1, value);
}


static void
fill_tables()
{
    fill_table1();
    fill_table2();
    fill_table3();
}

void
play_with_lookups(configuration* conf)
{
    struct table1_action* res = (struct table1_action*)table1_lookup(conf->tables, a1, a3);
    assert(res != NULL);
    assert(res->action_id == action_korte);
    assert(res->korte_params.i3 == 33);

    printf("LPM test passed.\n");

    uint8_t* res2 = table2_lookup(conf->tables, a1, a2);
    assert(res2 != NULL);
    assert(res2[0] == 4);
    assert(res2[1] == 5);
    assert(res2[2] == 6);
    uint8_t* res3 = table2_lookup(conf->tables, a1, a3); // DEFAULT_VAL!
    assert(res3 != NULL);
    assert(res3[0] == 44);
    assert(res3[1] == 55);
    assert(res3[2] == 66);
    
    printf("EXACT test passed.\n");

    uint8_t* res4 = table3_lookup(conf->tables, a1, a2);
    assert(res4 != NULL);
    assert(res4[0] == 7);
    assert(res4[1] == 8);
    assert(res4[2] == 9);
    uint8_t* res5 = table3_lookup(conf->tables, a1, a3);
    assert(res5 == NULL);

    printf("TERNARY test passed.\n");
}

int main(int argc, char** argv)
{
    printf("Hello DPDK Tables!\n");
    initialize(argc, argv);
    printf("Backend initialized.\n");
    printf("Fillig up the tables...\n");
    fill_tables();
    printf("Tables filled.\n");
    printf("\nTesting lookup...\n");
    play_with_lookups(&lcore_conf[0]);
    return 0;
}
