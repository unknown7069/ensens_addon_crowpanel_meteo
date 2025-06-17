#ifndef _PRESSURE_BOX_H_
#define _PRESSURE_BOX_H_

#include "common.h"

typedef struct {
    lv_obj_t*       cont;
    lv_obj_t*       title;
    meter_t*        gauge;
    meter_t*        tend;
    number_value_t* value;
    plot_t          plot;
} pressure_box_t;

pressure_box_t* pressure_box_create(lv_obj_t* parent);

#endif
