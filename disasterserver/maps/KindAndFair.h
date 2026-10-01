#ifndef KINDANDFAIR_H
#define KINDANDFAIR_H
#include "../Maps.h"

bool kaf_init		(Server* server);
bool kaf_tcpmsg	(PeerData* v, Packet* packet);

#endif