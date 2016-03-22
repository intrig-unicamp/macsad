#ifndef __TABLE_INFO_H__
#define __TABLE_INFO_H__

#include "dataplane.h"
#include "action_info.h"

lookup_table_t table_config[NB_TABLES] = {
    {
        .name = "table1",
        .type = LOOKUP_LPM,
        .key_size = 4,
        .val_size = sizeof(struct table1_action),
        .min_size = 0,
        .max_size = 255
    }
};

#endif
