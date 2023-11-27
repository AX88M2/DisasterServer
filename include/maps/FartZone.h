#ifndef FARTZONE_H
#define FARTZONE_H
#include "../Maps.h"

bool ft_init(Server* server);
bool ft_tcpmsg(PeerData* v, Packet* packet);

#endif