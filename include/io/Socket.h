#ifndef SOCKET_H
#define SOCKET_H

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

	#define SockError(signature) strncpy(signature, strerror(errno), 256)
	#define Poll poll
#else
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <WinSock2.h>
	#include <ws2tcpip.h>
	#include <mstcpip.h>
	#include <io.h>
	#include <errno.h>

	#define close closesocket
	#define SockError(signature) FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK , 0, WSAGetLastError(), 0, signature, 256, 0);
	#define Poll WSAPoll
	#define MSG_NOSIGNAL 0
    
    typedef int socklen_t;
#endif

#endif