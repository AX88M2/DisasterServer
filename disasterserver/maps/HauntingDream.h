#ifndef HAUNTDREAM_H
#define HAUNTDREAM_H
#include "../Maps.h"

bool hd_init(Server* server);
bool hd_tcpmsg(PeerData* v, Packet* packet);

#endif