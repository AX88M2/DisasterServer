#ifndef AUTH_H
#define AUTH_H
#include <Packet.h>
#include <Player.h>

// Override this struct in your auth provider
typedef struct
{
	uint32_t type;
	uint8_t	 one;
	uint8_t	 two;
} auth_peer_data;

typedef struct PeerData PeerData;

#endif
