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

#include "platform.h"
#include "netlib.h"

const netlib_version* netlib_get_version()
{
	static netlib_version version;
	NETLIB_VERSION(&version);
	return &version;
}

static int netlib_started = 0;

#ifndef WINDOWS
#include <signal.h>

int netlib_get_last_error(void)
{
	return errno;
}

void netlib_set_last_error(int err)
{
	errno = err;
}
#endif


static char errorbuf[1024];

void NETLIB_CALL netlib_set_error(const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsnprintf(errorbuf, sizeof(errorbuf), fmt, argp);
	va_end(argp);
}

const char* NETLIB_CALL netlib_get_error()
{
	return errorbuf;
}

int netlib_init(void)
{
	if (!netlib_started)
	{
#ifdef __USE_W32_SOCKETS
		/* Start up the windows networking */
		WORD version_wanted = MAKEWORD(1, 1);
		WSADATA wsaData;

		if (WSAStartup(version_wanted, &wsaData) != 0)
		{
			netlib_set_error("Couldn't initialize Winsock 1.1\n");
			return(-1);
		}
#else
		/* SIGPIPE is generated when a remote socket is closed */
		void(*handler)(int);
		handler = signal(SIGPIPE, SIG_IGN);
		if (handler != SIG_DFL)
			signal(SIGPIPE, handler);
#endif
	}

	++netlib_started;
	return 0;
}

void netlib_quit(void)
{
	if (netlib_started == 0)
		return;
	
	if (--netlib_started == 0)
	{
#ifdef __USE_W32_SOCKETS
		/* Clean up windows networking */
		if (WSACleanup() == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEINPROGRESS)
			{
#ifndef _WIN32_WCE
				WSACancelBlockingCall();
#endif
				WSACleanup();
			}
		}
#else
		/* Restore the SIGPIPE handler */
		void(*handler)(int);
		handler = signal(SIGPIPE, SIG_DFL);
		if (handler != SIG_IGN)
			signal(SIGPIPE, handler);
#endif
	}
}

int netlib_resolve_host(ip_address* address, const char* host, uint16_t port)
{
	int retval = 0;

	/* Perform the actual host resolution */
	if (host == NULL)
	{
		address->host = INADDR_ANY;
	}
	else
	{
		address->host = inet_addr(host);
		if (address->host == INADDR_NONE)
		{
			struct hostent *hp;
			hp = gethostbyname(host);
			if (hp)
				memcpy(&address->host, hp->h_addr, hp->h_length);
			else
				retval = -1;
		}
	}
	address->port = netlib_read16(&port);

	/* Return the status */
	return retval;
}

/* Resolve an ip address to a host name in canonical form.
   If the ip couldn't be resolved, this function returns NULL,
   otherwise a pointer to a static buffer containing the hostname
   is returned.  Note that this function is not thread-safe.

   Written by Miguel Angel Blanch.
   Main Programmer of Arianne RPG.
   http://come.to/arianne_rpg
*/
const char* netlib_resolve_ip(const ip_address* ip)
{
	struct hostent *hp;
	struct in_addr in;

	hp = gethostbyaddr((const char*) &ip->host, sizeof(ip->host), AF_INET);
	
	if (hp != NULL)
		return hp->h_name;

	in.s_addr = ip->host;
	return inet_ntoa(in);
}

int netlib_get_local_addresses(ip_address* addresses, int maxcount)
{
	int count = 0;
#ifdef SIOCGIFCONF
	/* Defined on Mac OS X */
#ifndef _SIZEOF_ADDR_IFREQ
#define _SIZEOF_ADDR_IFREQ sizeof
#endif
	SOCKET sock;
	struct ifconf conf;
	char data[4096];
	struct ifreq *ifr;
	struct sockaddr_in *sock_addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		return 0;
	}

	conf.ifc_len = sizeof(data);
	conf.ifc_buf = (caddr_t)data;
	if (ioctl(sock, SIOCGIFCONF, &conf) < 0) {
		closesocket(sock);
		return 0;
	}

	ifr = (struct ifreq*)data;
	while ((char*)ifr < data + conf.ifc_len) {
		if (ifr->ifr_addr.sa_family == AF_INET) {
			if (count < maxcount) {
				sock_addr = (struct sockaddr_in*)&ifr->ifr_addr;
				addresses[count].host = sock_addr->sin_addr.s_addr;
				addresses[count].port = sock_addr->sin_port;
			}
			++count;
		}
		ifr = (struct ifreq*)((char*)ifr + _SIZEOF_ADDR_IFREQ(*ifr));
	}
	closesocket(sock);
#elif defined(WIN32)
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter;
	PIP_ADDR_STRING pAddress;
	DWORD dwRetVal = 0;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	pAdapterInfo = (IP_ADAPTER_INFO*) malloc(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL)
		return 0;
	
	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == ERROR_BUFFER_OVERFLOW)
	{
		pAdapterInfo = (IP_ADAPTER_INFO*) realloc(pAdapterInfo, ulOutBufLen);
		if (pAdapterInfo == NULL)
			return 0;

		dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	}

	if (dwRetVal == NO_ERROR)
	{
		for (pAdapter = pAdapterInfo; pAdapter; pAdapter = pAdapter->Next)
		{
			for (pAddress = &pAdapter->IpAddressList; pAddress; pAddress = pAddress->Next)
			{
				if (count < maxcount)
				{
					addresses[count].host = inet_addr(pAddress->IpAddress.String);
					addresses[count].port = 0;
				}
				++count;
			}
		}
	}
	free(pAdapterInfo);
#endif
	return count;
}