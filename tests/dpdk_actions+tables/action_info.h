#ifndef __ACTION_INFO_H__
#define __ACTION_INFO_H__

enum actions {
    action_alma,
    action_korte,
};

struct action_alma_params {
    int i1;
};

struct action_korte_params {
    int i1;
    int i2;
    int i3;
};

struct table1_action {
    int action_id;
    union {
        struct action_alma_params  alma_params;
        struct action_korte_params korte_params;
    };
};

#endif
