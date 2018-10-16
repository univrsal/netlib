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
 * Source: libsdl.org/projects/SDL_net/docs/demos/tcpclient.c
 */

#include <stdio.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <string.h>
#include <netlib.h>

int main(int argc, char **argv)
{
	ip_address ip;
	tcp_socket sock;
	char message[1024];
	int len;
	uint16_t port;

	/* check our commandline */
	if (argc<3)
	{
		printf("usage:\n [host] [port]\n", argv[0]);
		exit(0);
	}

	/* initialize netlib */
	if (netlib_init() == -1)
	{
		printf("netlib_init: %s\n", netlib_get_error());
		exit(2);
	}

	/* get the port from the commandline */
	port = (uint16_t)strtol(argv[2], NULL, 0);

	/* Resolve the argument into an IPaddress type */
	if (netlib_resolve_host(&ip, argv[1], port) == -1)
	{
		printf("netlib_resolve_host: %s\n", netlib_get_error());
		exit(3);
	}

	/* open the server socket */
	sock = netlib_tcp_open(&ip);
	if (!sock)
	{
		printf("netlib_tcp_open: %s\n", netlib_get_error());
		exit(4);
	}

	/* read the buffer from stdin */
	printf("Enter Message, or Q to make the server quit:\n");
	fgets(message, 1024, stdin);
	len = strlen(message);

	/* strip the newline */
	message[len - 1] = '\0';

	if (len)
	{
		int result;

		/* print out the message */
		printf("Sending: %.*s\n", len, message);

		result = netlib_tcp_send(sock, message, len); /* add 1 for the NULL */
		if (result<len)
			printf("netlib_tcp_send: %s\n", netlib_get_error());
	}

	netlib_tcp_close(sock);

	/* shutdown netlib */
	netlib_quit();

	return 0;
}