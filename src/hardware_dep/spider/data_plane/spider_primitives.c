#include "shared_primitives.c"
#include "backend.h"
#include "dataplane.h"

#include <string.h> // memcpy

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

char*
memory_prepend(void* p, uint8_t size)
{
    return pkthdr_encap(size);
}

void
memory_adj(void* p, uint8_t size)
{
	pkthdr_decap(size);
}

void
hardware_dep_memcpy(void* dst,  void* src, uint8_t size){
    spad_copy(dst, src, size);
    pkthdr_mark_dirty(dst, size);
}

void 
generate_digest(backend bg, char* name, int receiver, struct type_field_list* digest_field_list)
{
    /*TODO*/
}

void copy_header(packet* p1, packet* p2, header_idx h1, header_idx h2, length l)
{
	spad_copy(h1, (uint8_t /*__spad*/*) h2, l);
}

void drop(packet* p)
{
	packet_drop(PACKET_DROP_REASON_DEFAULT, PACKET_DROP_TYPE_DEFAULT);
}
