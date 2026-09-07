#ifndef NASTYPUSSY_H
#define NASTYPUSSY_H
#include "../Maps.h"

bool nap_init(Server* server);
bool nap_tcpmsg(PeerData* v, Packet* packet);

#endif