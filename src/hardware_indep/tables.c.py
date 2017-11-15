
from utils.misc import addError, addWarning

#[ #include "dataplane.h"
#[ #include "actions.h"
#[ #include "data_plane_data.h"
#[

#[ lookup_table_t table_config[NB_TABLES] = {
for table in hlir16.tables:
    #[ {
    #[  .name= "${table.name}",
    #[  .id = TABLE_${table.name},
    #[  .type = LOOKUP_${table.match_type},
    #[  .key_size = ${table.key_length_bytes},
    #[  .val_size = sizeof(struct ${table.name}_action),
    #[  .min_size = 0,
    #[  .max_size = 255,
    #[ },
#[ };

#[ counter_t counter_config[NB_COUNTERS] = {
#[     // TODO feature temporarily not supported (hlir16)
#[ };

#[ p4_register_t register_config[NB_REGISTERS] = {
#[     // TODO feature temporarily not supported (hlir16)
#[ };
