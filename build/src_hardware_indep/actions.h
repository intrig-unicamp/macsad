 #ifndef __ACTION_INFO_GENERATED_H__// sugar@3
 #define __ACTION_INFO_GENERATED_H__// sugar@4
 

 #define FIELD(name, length) uint8_t name[(length + 7) / 8];// sugar@7

 enum actions {// sugar@10
 action_mac_learn,// sugar@16
 action__nop,// sugar@16
 action_forward,// sugar@16
 action_bcast,// sugar@16
 };// sugar@17
 struct action_forward_params {// sugar@22
 FIELD(port, 9);// sugar@24
 };// sugar@25
 struct smac_action {// sugar@28
     int action_id;// sugar@29
     union {// sugar@30
     };// sugar@34
 };// sugar@35
 struct dmac_action {// sugar@28
     int action_id;// sugar@29
     union {// sugar@30
 struct action_forward_params forward_params;// sugar@33
     };// sugar@34
 };// sugar@35
 void action_code_mac_learn(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@42
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@42
 void action_code_forward(packet_descriptor_t *pd, lookup_table_t **tables, struct action_forward_params);// sugar@40
 void action_code_bcast(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@42
 #endif// sugar@44
