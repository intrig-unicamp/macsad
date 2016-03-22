 #include "odp_lib.h"// sugar@8
 #include "actions.h"// sugar@9
 #include <unistd.h>// sugar@10
 #include <arpa/inet.h>// sugar@11
 extern backend bg;// sugar@141

  void action_code__nop(packet_descriptor_t* pkt, lookup_table_t** tables ) {// sugar@151
     uint32_t value32, res32;// sugar@152
     (void)value32;// sugar@153
     (void)res32;// sugar@154
 }// sugar@171

  void action_code_mac_learn(packet_descriptor_t* pkt, lookup_table_t** tables ) {// sugar@151
     uint32_t value32, res32;// sugar@152
     (void)value32;// sugar@153
     (void)res32;// sugar@154
  struct type_field_list fields;// sugar@118
    fields.fields_quantity = 2;// sugar@120
    fields.field_offsets = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@121
    fields.field_widths = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@122
    fields.field_offsets[0] = (uint8_t*) (pkt->headers[header_instance_ethernet].pointer + field_instance_byte_offset_hdr[field_instance_ethernet_srcAddr]);// sugar@126
    fields.field_widths[0] = field_instance_bit_width[field_instance_ethernet_srcAddr]*8;// sugar@127
    fields.field_offsets[1] = (uint8_t*) (pkt->headers[header_instance_standard_metadata].pointer + field_instance_byte_offset_hdr[field_instance_standard_metadata_ingress_port]);// sugar@126
    fields.field_widths[1] = field_instance_bit_width[field_instance_standard_metadata_ingress_port]*8;// sugar@127

    generate_digest(bg,"mac_learn_digest",0,&fields); sleep(1);// sugar@133
 }// sugar@171

  void action_code_forward(packet_descriptor_t* pkt, lookup_table_t** tables , struct action_forward_params parameters) {// sugar@151
     uint32_t value32, res32;// sugar@152
     (void)value32;// sugar@153
     (void)res32;// sugar@154
  MODIFY_INT32_BYTEBUF(pkt, field_instance_standard_metadata_egress_port, parameters.port, 2)// sugar@55
// sugar@159
 }// sugar@171

  void action_code_bcast(packet_descriptor_t* pkt, lookup_table_t** tables ) {// sugar@151
     uint32_t value32, res32;// sugar@152
     (void)value32;// sugar@153
     (void)res32;// sugar@154
  value32 = 100;// sugar@26
 MODIFY_INT32_INT32(pkt, field_instance_standard_metadata_egress_port, value32)// sugar@30
// sugar@159
 }// sugar@171

