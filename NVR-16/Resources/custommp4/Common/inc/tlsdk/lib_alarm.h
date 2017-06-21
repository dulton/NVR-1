
#ifndef __LIB_ALARM_H
#define __LIB_ALARM_H

#include "lib_common.h"

//return value: bit[0]->in0, bit[1]->in1, ...
//bit[n] value means pin level
unsigned int tl_get_alarm_input(void);

//return: 0->ok, other fail
int tl_set_alarm_out(int channel, int val);

#endif
