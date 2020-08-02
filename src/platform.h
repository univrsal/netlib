/**
 * This file is part of netlib
 * which is licensed under the zlib license
 *
 * github.com/univrsal/netlib
 * netlib is a small network library
 * heavily based on SDL_net without
 * the dependency on SDL
 *
 * Documentation is directly taken from SDL_net
 * I take no credit for this code, I only
 * reformatted it for my needs and made slight changes
 */

#ifndef NETLIB_PLATFORM_H
#define NETLIB_PLATFORM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <errno.h>

#if defined(_WIN32) || defined(_WIN64)
#define __USE_W32_SOCKETS
#ifdef _WIN64
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <winsock.h>
/* NOTE: windows socklen_t is signed
 * and is defined only for winsock2. */
typedef int socklen_t;
#endif /* W64 */
#include <iphlpapi.h>
#else /* UNIX */
#include <sys/types.h>
#ifdef __FreeBSD__
#include <sys/socket.h>
#endif
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#ifndef __BEOS__
#include <arpa/inet.h>
#endif
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#endif /* WIN32 */

#ifndef __USE_W32_SOCKETS
#ifdef __OS2__
#define closesocket     soclose
#else  /* !__OS2__ */
#define closesocket close
#endif /* __OS2__ */
#define SOCKET  int
#define INVALID_SOCKET  -1
#define SOCKET_ERROR    -1
#endif /* __USE_W32_SOCKETS */

#ifdef __USE_W32_SOCKETS
#define netlib_get_last_error WSAGetLastError
#define netlib_set_last_error WSASetLastError
#ifndef EINTR
#define EINTR WSAEINTR
#endif
#else
int netlib_get_last_error(void);
void netlib_set_last_error(int err);
#endif

#endif
