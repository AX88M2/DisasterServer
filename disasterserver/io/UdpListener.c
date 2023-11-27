#include "io/UdpListener.h"
#include "io/TcpListener.h"

#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)

bool udp_open(UdpListener* udp, uint16_t port)
{
	RAssert(udp);

	udp->fd = (int)socket(AF_INET, SOCK_DGRAM, 0);
	RAssert(udp->fd >= 0);
	
	// Init listener
	memset(&udp->addr, 0, sizeof(udp->addr));
	udp->addr.sin_family = AF_INET;
	udp->addr.sin_addr.s_addr = INADDR_ANY;
	udp->addr.sin_port = htons(port);

	RAssert(bind(udp->fd, (struct sockaddr*)&udp->addr, sizeof(udp->addr)) >= 0);
	Info("%d SOCK_DGRAM (port %d) bind succeeded.", udp->fd, ntohs(udp->addr.sin_port));

#ifdef WIN32
	BOOL beh = FALSE;
	DWORD returned = 0;
	RAssert(WSAIoctl(udp->fd, SIO_UDP_CONNRESET, &beh, sizeof(beh), NULL, 0, &returned, NULL, NULL) == 0);
	Info("%d SOCK_DGRAM (port %d) applied SIO_UDP_CONNRESET hack.", udp->fd, ntohs(udp->addr.sin_port));
#endif

	return true;
}