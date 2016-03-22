 // TODO: insert the parser_exception calls into right places// sugar@38
 #include "odp_lib.h"// sugar@64
 void print_mac(uint8_t* v) { printf("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n", v[0], v[1], v[2], v[3], v[4], v[5]); }// sugar@65
 void print_ip(uint8_t* v) { printf("%d.%d.%d.%d\n",v[0],v[1],v[2],v[3]); }// sugar@66

 static inline void p4_pe_header_too_short(packet_descriptor_t *pd) {// sugar@71
     drop(pd->packet);// sugar@73
 }// sugar@78
 static inline void p4_pe_default(packet_descriptor_t *pd) {// sugar@71
     drop(pd->packet);// sugar@73
 }// sugar@78
 static inline void p4_pe_checksum(packet_descriptor_t *pd) {// sugar@71
     drop(pd->packet);// sugar@73
 }// sugar@78
 static inline void p4_pe_unhandled_select(packet_descriptor_t *pd) {// sugar@71
     drop(pd->packet);// sugar@73
 }// sugar@78
 static inline void p4_pe_index_out_of_bounds(packet_descriptor_t *pd) {// sugar@71
     drop(pd->packet);// sugar@73
 }// sugar@78
 static inline void p4_pe_header_too_long(packet_descriptor_t *pd) {// sugar@71
     drop(pd->packet);// sugar@73
 }// sugar@78
 static inline void p4_pe_out_of_packet(packet_descriptor_t *pd) {// sugar@71
     drop(pd->packet);// sugar@73
 }// sugar@78

 void apply_table_smac(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@83
 void apply_table_dmac(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@83
 
 static void// sugar@85
 extract_header(uint8_t* buf, packet_descriptor_t* pd, header_instance_t h) {// sugar@86
     pd->headers[h] =// sugar@87
       (header_descriptor_t) {// sugar@88
         .type = h,// sugar@89
         .pointer = buf,// sugar@90
         .length = header_instance_byte_width[h]// sugar@91
       };// sugar@92
 }// sugar@93
 
 static void parse_state_start(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables);// sugar@97
 static void parse_state_parse_ethernet(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables);// sugar@97

 static void parse_state_start(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables)// sugar@129
 {// sugar@130
     return parse_state_parse_ethernet(pd, buf, tables);// sugar@157
 }// sugar@221
 
 static void parse_state_parse_ethernet(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables)// sugar@129
 {// sugar@130
     extract_header(buf, pd, header_instance_ethernet);// sugar@135
     buf += header_instance_byte_width[header_instance_ethernet];// sugar@136
     #if debug == 1// sugar@159
     printf("  :::: PACKET PARSED\n");// sugar@160
     printf("    :: ethernet srcaddr: ");// sugar@161
     print_mac(extract_bytebuf(pd, field_desc(field_instance_ethernet_srcAddr)));// sugar@162
     printf("    :: ethernet dstaddr: ");// sugar@163
     print_mac(extract_bytebuf(pd, field_desc(field_instance_ethernet_dstAddr)));// sugar@164
     //printf("    :: IP src: "); // sugar@165
     //print_ip(extract_bytebuf(pd, field_desc(field_instance_ipv4_srcAddr))); // sugar@166
     //printf("    :: IP dst: "); // sugar@167
     //print_ip(extract_bytebuf(pd, field_desc(field_instance_ipv4_dstAddr)));// sugar@168
     for (int i = 0; i < HEADER_INSTANCE_COUNT; ++i) {// sugar@169
         printf("    :: header %d (type=%d, len=%d) = ", i, pd->headers[i].type, pd->headers[i].length);// sugar@170
         for (int j = 0; j < pd->headers[i].length; ++j) {// sugar@171
         printf("%02x ", ((uint8_t*)(pd->headers[i].pointer))[j]);// sugar@172
         }// sugar@173
         printf("\n");// sugar@174
     }// sugar@175

     printf("  :::: TURNING TO TABLE smac\n");// sugar@177
     #endif// sugar@178
     return apply_table_smac(pd, tables);// sugar@179
 }// sugar@221
 
 void init_metadata_headers(packet_descriptor_t* packet_desc) {// sugar@228
 }// sugar@234

 void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@237
     init_metadata_headers(pd);// sugar@238
     parse_state_start(pd, pd->pointer, tables);// sugar@239
 }// sugar@240
