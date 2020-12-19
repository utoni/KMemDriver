#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <iostream>
#include <thread>

#include "CheatEngine.h"
#include "CommandDispatcher.h"

static SOCKET sock;
static BOOL run_main_loop = TRUE;

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

static int accept_loop(const char* servspec) {
	sock = make_accept_sock(servspec);

	if (sock == NULL)
	{
		return 1;
	}

	while (run_main_loop == TRUE) {
		SOCKET new_sock = accept(sock, 0, 0);
		if (new_sock != NULL) {
			std::thread t(new_connection, new_sock);
			t.detach();
		}
		else {
			return 1;
		}
	}
	return 0;
}

static void onPingThreadTimeout(void) {
	std::cout << "PingThread timeout, abort .." << std::endl;
	run_main_loop = FALSE;
	closesocket(sock);
}

int main()
{
	WSADATA wsaData;
	DWORD iResult;
	KInterface& ki = KInterface::getInstance();

	std::cout << "KMemDriver Init/Handshake.";
	if (ki.Init() == false || ki.Handshake() == false) {
		std::cout << " Failed. [PRESS RETURN TO EXIT]" << std::endl;
		getchar();
		return 1;
	}
	std::cout << " Ok." << std::endl;

	ki.StartPingThread(onPingThreadTimeout);

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << "\n";
		return 1;
	}

	return accept_loop("0.0.0.0");
}