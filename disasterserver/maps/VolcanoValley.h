#ifndef VOLCANOVALLEY_H
#define VOLCANOVALLEY_H
#include "../Maps.h"

bool vv_init(Server* server);
bool vv_tcpmsg(PeerData* v, Packet* packet);

#endif