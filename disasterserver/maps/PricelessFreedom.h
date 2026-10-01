#ifndef PRICELESSFREEDOM_H
#define PRICELESSFREEDOM_H
#include "../Maps.h"

bool pf_init(Server* server);
bool pf_tcpmsg(PeerData* v, Packet* packet);

#endif