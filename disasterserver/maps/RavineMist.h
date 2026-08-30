#ifndef RAVINEMIST_H
#define RAVINEMIST_H
#include "../Maps.h"

bool rmz_init(Server* server);
bool rmz_tick(Server* server);
bool rmz_tcpmsg(PeerData* v, Packet* packet);
bool rmz_left(PeerData* v);

#endif