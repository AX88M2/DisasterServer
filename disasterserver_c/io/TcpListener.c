#include <io/TcpListener.h>

bool tcp_open(TcpListener* tcp, uint16_t port)
{
	RAssert(tcp);

#ifdef WIN32
	WSADATA wsaData;
	RAssert(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
#endif

	tcp->fd = (int)socket(AF_INET, SOCK_STREAM, 0);
	RAssert(tcp->fd >= 0);

	const int opt = 1;
	RAssert(setsockopt(tcp->fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt)) >= 0);
	RAssert(setsockopt(tcp->fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) >= 0);

	// Init listener
	memset(&tcp->addr, 0, sizeof(tcp->addr));
	tcp->addr.sin_family = AF_INET;
	tcp->addr.sin_addr.s_addr = INADDR_ANY;
	tcp->addr.sin_port = htons(port);

	RAssert(bind(tcp->fd, (struct sockaddr*)&tcp->addr, sizeof(tcp->addr)) >= 0);
	Info("%d SOCK_STREAM (port %d) bind succeeded.", tcp->fd, ntohs(tcp->addr.sin_port));
	return true;
}

bool tcp_listen(TcpListener* tcp)
{
	RAssert(tcp);
	RAssert(listen(tcp->fd, 3) >= 0);
	Info("%d SOCK_STREAM (port %d) listening for incoming connections.", tcp->fd, ntohs(tcp->addr.sin_port));
	
	return true;
}

bool tcp_next(TcpListener* tcp, int* id, struct sockaddr_in* addr)
{
	RAssert(tcp);
	
	size_t addr_len = sizeof(struct sockaddr_in);
	*id = accept(tcp->fd, (struct sockaddr*)addr, &addr_len);

	if (*id < 0)
	{
		char err[256];
		SOCKERR(err);
		Err("%d SOCK_STREAM: accept failed for fd %d: %s", tcp->fd, *id, err);

		return false;
	}

#ifdef WIN32
	static const u_long mode = 1;  // 1 to enable non-blocking socket

	if (ioctlsocket(*id, FIONBIO, &mode) != NOERROR)
	{
		char err[256];
		SOCKERR(err);
		Err("%d SOCK_STREAM: fnctl failed for fd %d: %s", tcp->fd, *id, err);

		close(*id);
		return false;
	}
#elif defined(__unix) || defined(__unix__)
	if (fcntl(*id, F_SETFL, fcntl(*id, F_GETFL, 0) | O_NONBLOCK) == -1)
	{
		char err[256];
		SOCKERR(err);
		Err("%d SOCK_STREAM: fnctl failed for fd %d: %s", tcp->fd, *id, err);

		close(*id);
		return false;
	}
#endif

	Info("%d SOCK_STREAM: connected (id %d, ip %s)", tcp->fd, *id, inet_ntoa(addr->sin_addr));
	return true;
}

bool tcp_kill(TcpListener* tcp, int id)
{
	close(id);
	RAssert(tcp);

	Info("%d SOCK_STREAM: disconnected (id %d)", tcp->fd, id);
	return true;
}