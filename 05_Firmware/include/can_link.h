#pragma once
#include "protocol.h"
void can_link_init(void);
bool can_link_send(Reading reading);
bool can_link_receive(Reading *reading);
typedef struct { uint32_t queued, dropped, received, invalid, bus_off_events; } CanStats;
extern CanStats can_stats;
