#ifndef LIMPCITY_H
#define LIMPCITY_H
#include "../Maps.h"

bool lc_init(Server* server);
bool lc_tcpmsg(PeerData* v, Packet* packet);

#endif