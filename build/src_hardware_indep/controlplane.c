 #include "odp_lib.h"// sugar@12
 #include "actions.h"// sugar@13
 
 extern void table_setdefault_promote  (int tableid, uint8_t* value);// sugar@15
 extern void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value);// sugar@16
 extern void lpm_add_promote    (int tableid, uint8_t* key, uint8_t depth, uint8_t* value);// sugar@17
 extern void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value);// sugar@18

 extern void table_smac_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@21
 extern void table_dmac_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@21
 
 void// sugar@26
 smac_add(// sugar@27
 uint8_t field_instance_ethernet_srcAddr[6],// sugar@30
 struct smac_action action)// sugar@36
 {// sugar@37
     uint8_t key[6];// sugar@38
 memcpy(key+0, field_instance_ethernet_srcAddr, 6);// sugar@43
 exact_add_promote(TABLE_smac, (uint8_t*)key, (uint8_t*)&action);// sugar@57
 }// sugar@58

 void// sugar@60
 smac_setdefault(struct smac_action action)// sugar@61
 {// sugar@62
     table_setdefault_promote(TABLE_smac, (uint8_t*)&action);// sugar@63
 }// sugar@64
 void// sugar@26
 dmac_add(// sugar@27
 uint8_t field_instance_ethernet_dstAddr[6],// sugar@30
 struct dmac_action action)// sugar@36
 {// sugar@37
     uint8_t key[6];// sugar@38
 memcpy(key+0, field_instance_ethernet_dstAddr, 6);// sugar@43
 exact_add_promote(TABLE_dmac, (uint8_t*)key, (uint8_t*)&action);// sugar@57
 }// sugar@58

 void// sugar@60
 dmac_setdefault(struct dmac_action action)// sugar@61
 {// sugar@62
     table_setdefault_promote(TABLE_dmac, (uint8_t*)&action);// sugar@63
 }// sugar@64
 void// sugar@68
 smac_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@69
 uint8_t* field_instance_ethernet_srcAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@73
 if(strcmp("mac_learn", ctrl_m->action_name)==0) {// sugar@78
     struct smac_action action;// sugar@79
     action.action_id = action_mac_learn;// sugar@80
     printf("Reply from the control plane arrived.\n");// sugar@86
     printf("Addig new entry to smac with action mac_learn\n");// sugar@87
     smac_add(// sugar@88
 field_instance_ethernet_srcAddr,// sugar@91
     action);// sugar@94
 }// sugar@95
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@78
     struct smac_action action;// sugar@79
     action.action_id = action__nop;// sugar@80
     printf("Reply from the control plane arrived.\n");// sugar@86
     printf("Addig new entry to smac with action _nop\n");// sugar@87
     smac_add(// sugar@88
 field_instance_ethernet_srcAddr,// sugar@91
     action);// sugar@94
 }// sugar@95
 }// sugar@96
 void// sugar@68
 dmac_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@69
 uint8_t* field_instance_ethernet_dstAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@73
 if(strcmp("forward", ctrl_m->action_name)==0) {// sugar@78
     struct dmac_action action;// sugar@79
     action.action_id = action_forward;// sugar@80
 action.forward_params;// sugar@82
 uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@84
 memcpy(action.forward_params.port, port, 2);// sugar@85
     printf("Reply from the control plane arrived.\n");// sugar@86
     printf("Addig new entry to dmac with action forward\n");// sugar@87
     dmac_add(// sugar@88
 field_instance_ethernet_dstAddr,// sugar@91
     action);// sugar@94
 }// sugar@95
 if(strcmp("bcast", ctrl_m->action_name)==0) {// sugar@78
     struct dmac_action action;// sugar@79
     action.action_id = action_bcast;// sugar@80
     printf("Reply from the control plane arrived.\n");// sugar@86
     printf("Addig new entry to dmac with action bcast\n");// sugar@87
     dmac_add(// sugar@88
 field_instance_ethernet_dstAddr,// sugar@91
     action);// sugar@94
 }// sugar@95
 }// sugar@96
 void recv_from_controller(struct p4_ctrl_msg* ctrl_m) {// sugar@98
     if (ctrl_m->type == P4T_ADD_TABLE_ENTRY) {// sugar@99
 if (strcmp("smac", ctrl_m->table_name) == 0)// sugar@101
     smac_add_table_entry(ctrl_m);// sugar@102
 if (strcmp("dmac", ctrl_m->table_name) == 0)// sugar@101
     dmac_add_table_entry(ctrl_m);// sugar@102
     }// sugar@103
 }// sugar@104
 backend bg;// sugar@108
 void init_control_plane()// sugar@109
 {// sugar@110
     printf("Creating control plane connection...\n");// sugar@111
     bg = create_backend(3, 1000, "localhost", 11111, recv_from_controller);// sugar@112
     launch_backend(bg);// sugar@113
 struct smac_action action;// sugar@115
 action.action_id = action_mac_learn;// sugar@116
 smac_setdefault(action);// sugar@117
 printf("smac setdefault\n");// sugar@118
 struct dmac_action action2;// sugar@120
 action2.action_id = action_bcast;// sugar@121
 dmac_setdefault(action2);// sugar@122
 printf("dmac setdefault\n");// sugar@123
 }// sugar@124
