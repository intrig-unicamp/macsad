 #include <stdlib.h>// sugar@7
 #include <string.h>// sugar@8
 #include "odp_lib.h"// sugar@9
 #include "actions.h"// sugar@10
 
 
 extern void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@16

 void apply_table_smac(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@19
 void apply_table_dmac(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@19

 void table_smac_key(packet_descriptor_t* pd, uint8_t* key) {// sugar@28
     field_reference_t f;// sugar@29
     (void)f;// sugar@30
 EXTRACT_BYTEBUF(pd, field_instance_ethernet_srcAddr, key)// sugar@38
 key += 6;// sugar@39
 }// sugar@42

 void table_dmac_key(packet_descriptor_t* pd, uint8_t* key) {// sugar@28
     field_reference_t f;// sugar@29
     (void)f;// sugar@30
 EXTRACT_BYTEBUF(pd, field_instance_ethernet_dstAddr, key)// sugar@38
 key += 6;// sugar@39
 }// sugar@42

 void apply_table_smac(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@48
 {// sugar@49
     #if debug == 1// sugar@50
     printf("  :::: EXECUTING TABLE smac\n");// sugar@51
     #endif// sugar@52
     uint8_t* key[6];// sugar@53
     table_smac_key(pd, (uint8_t*)key);// sugar@54
     struct smac_action* res = (struct smac_action*)exact_lookup(tables[TABLE_smac], (uint8_t*)key);// sugar@55
     if(res == NULL) {// sugar@56
         #if debug == 1// sugar@57
         printf("    :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET... key:0x%08x\n", (unsigned)*key);// sugar@58
         #endif// sugar@59
         return;// sugar@60
     }// sugar@61
     switch (res->action_id) {// sugar@62
 case action_mac_learn:// sugar@64
   #if debug == 1// sugar@65
   printf("    :: EXECUTING ACTION mac_learn...\n");// sugar@66
   #endif// sugar@67
 action_code_mac_learn(pd, tables);// sugar@71
     break;// sugar@72
 case action__nop:// sugar@64
   #if debug == 1// sugar@65
   printf("    :: EXECUTING ACTION _nop...\n");// sugar@66
   #endif// sugar@67
 action_code__nop(pd, tables);// sugar@71
     break;// sugar@72
     }// sugar@73
 switch (res->action_id) {// sugar@78
 case action_mac_learn:// sugar@81
     apply_table_dmac(pd, tables);// sugar@82
     break;// sugar@83
 case action__nop:// sugar@81
     apply_table_dmac(pd, tables);// sugar@82
     break;// sugar@83
 }// sugar@84
 }// sugar@85

 void apply_table_dmac(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@48
 {// sugar@49
     #if debug == 1// sugar@50
     printf("  :::: EXECUTING TABLE dmac\n");// sugar@51
     #endif// sugar@52
     uint8_t* key[6];// sugar@53
     table_dmac_key(pd, (uint8_t*)key);// sugar@54
     struct dmac_action* res = (struct dmac_action*)exact_lookup(tables[TABLE_dmac], (uint8_t*)key);// sugar@55
     if(res == NULL) {// sugar@56
         #if debug == 1// sugar@57
         printf("    :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET... key:0x%08x\n", (unsigned)*key);// sugar@58
         #endif// sugar@59
         return;// sugar@60
     }// sugar@61
     switch (res->action_id) {// sugar@62
 case action_forward:// sugar@64
   #if debug == 1// sugar@65
   printf("    :: EXECUTING ACTION forward...\n");// sugar@66
   #endif// sugar@67
 action_code_forward(pd, tables, res->forward_params);// sugar@69
     break;// sugar@72
 case action_bcast:// sugar@64
   #if debug == 1// sugar@65
   printf("    :: EXECUTING ACTION bcast...\n");// sugar@66
   #endif// sugar@67
 action_code_bcast(pd, tables);// sugar@71
     break;// sugar@72
     }// sugar@73
 switch (res->action_id) {// sugar@78
 }// sugar@84
 }// sugar@85

/* 
 void handle_packet(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@89
 {// sugar@90
     #if debug == 1// sugar@91
     printf("### HANDLING PACKET ARRIVING AT PORT %" PRIu32 "...\n", extract_intvalue(pd, field_desc(field_instance_standard_metadata_ingress_port)));// sugar@92
     #endif// sugar@93
     parse_packet(pd, tables);// sugar@94
 }// sugar@95
*/ 
void handle_packet(packet_descriptor_t* pd)// sugar@89
{
	printf("handle packet called");
}
