 #include "dataplane.h"// sugar@3
 #include "actions.h"// sugar@4
 #include "data_plane_data.h"// sugar@5

 lookup_table_t table_config[NB_TABLES] = {// sugar@7
 {// sugar@10
  .name= "smac",// sugar@11
  .type = LOOKUP_EXACT,// sugar@12
  .key_size = 6,// sugar@13
  .val_size = sizeof(struct smac_action),// sugar@14
  .min_size = 0, //512,// sugar@15
  .max_size = 255 //512// sugar@16
 },// sugar@17
 {// sugar@10
  .name= "dmac",// sugar@11
  .type = LOOKUP_EXACT,// sugar@12
  .key_size = 6,// sugar@13
  .val_size = sizeof(struct dmac_action),// sugar@14
  .min_size = 0, //512,// sugar@15
  .max_size = 255 //512// sugar@16
 },// sugar@17
 };// sugar@18
