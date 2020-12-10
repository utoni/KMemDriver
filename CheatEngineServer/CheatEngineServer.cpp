#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <iostream>
#include <thread>

#include "CheatEngine.h"
#include "CommandDispatcher.h"

static SOCKET make_accept_sock(const char* servspec) {
	const int one = 1;
	struct addrinfo hints = {};
	struct addrinfo* res = 0, * ai = 0, * ai4 = 0;
	SOCKET sock;

	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (servspec == NULL)
	{
		servspec = "0::0";
	}
	std::cout << "Listen on " << servspec << ":" << CE_PORT << std::endl;
	getaddrinfo(servspec, CE_PORT, &hints, &res);

	for (ai = res; ai; ai = ai->ai_next) {
		if (ai->ai_family == PF_INET6) break;
		else if (ai->ai_family == PF_INET) ai4 = ai;
	}
	ai = ai ? ai : ai4;

	if (ai == NULL) {
		return NULL;
	}

	sock = socket(ai->ai_family, SOCK_STREAM, 0);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one));
	if (bind(sock, ai->ai_addr, (int)ai->ai_addrlen) != 0)
	{
		std::cout << "bind() failed" << std::endl;
	}
	if (listen(sock, 256) != 0)
	{
		std::cout << "listen() failed" << std::endl;
	}
	freeaddrinfo(res);

	return sock;
}

static void new_connection(SOCKET sock) {
	CEConnection cec(sock);
	std::cout << "New connection .." << std::endl;
	while (1) {
		if (CheckForAndDispatchCommand(cec) != 0)
		{
			std::cout << "Closing connection .." << std::endl;
			cec.closeSocket();
			break;
		}
	}
}

static void accept_loop(const char* servspec) {
	SOCKET sock = make_accept_sock(servspec);

	if (sock == NULL)
	{
		return;
	}

	for (;;) {
		SOCKET new_sock = accept(sock, 0, 0);
		new_connection(new_sock);
		//std::thread t(new_connection, new_sock);
		//t.detach();
	}
}

int main()
{
	WSADATA wsaData;
	DWORD iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << "\n";
		return 1;
	}

	accept_loop("0.0.0.0");
}