#ifndef TCPLISTENER_H
#define TCPLISTENER_H

#include "../Log.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__unix) || defined(__unix__)
	#include <arpa/inet.h>
	#include <netinet/tcp.h>
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <poll.h>
	#include <string.h>
	#include <ctype.h>
	#include <errno.h>

	#define SOCKERR(signature) strncpy(signature, strerror(errno), 256)
	#define Poll poll
#else
	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <Windows.h>
	#include <ws2tcpip.h>
	#include <mstcpip.h>
	#include <io.h>
	#include <errno.h>

	#define close closesocket
	#define SOCKERR(signature) FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK , 0, WSAGetLastError(), 0, signature, 256, 0);
	#define Poll WSAPoll
	#define MSG_NOSIGNAL 0
#endif

typedef struct
{
	struct sockaddr_in	addr;
	int					fd;
} TcpListener;

bool	tcp_open	(TcpListener* tcp, uint16_t port);
bool	tcp_listen	(TcpListener* tcp);
bool	tcp_next	(TcpListener* tcp, int* fd, struct sockaddr_in* addr);
bool	tcp_kill	(TcpListener* tcp, int fd);

#endif
