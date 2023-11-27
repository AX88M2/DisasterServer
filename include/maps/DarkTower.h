#ifndef DARKTOWER_H
#define DARKTOWER_H
#include "../Maps.h"

bool dt_init(Server* server);
bool dt_tcpmsg(PeerData* v, Packet* packet);

#endif