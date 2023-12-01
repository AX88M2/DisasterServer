#ifndef UDPLISTENER_H
#define UDPLISTENER_H

#include "../Log.h"

#if defined(__unix) || defined(__unix__)
	#include <arpa/inet.h>
	#include <netinet/tcp.h>
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <unistd.h>
#else
	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <Windows.h>
	//#define close closesocket
#endif

typedef union
{
	struct sockaddr addr;
	struct sockaddr_in v4;
	struct sockaddr_storage storage;
} IpAddr;

typedef struct
{
	struct sockaddr_in	addr;
	int					fd;
} UdpListener;

bool udp_open		(UdpListener* udp, uint16_t port);

#endif
