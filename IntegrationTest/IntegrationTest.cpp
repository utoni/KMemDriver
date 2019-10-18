// IntegrationTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "KInterface.h"

#include <iostream>

int main()
{
	std::cout << "IntegrationTest\n";

	try {
		KInterface &ki = KInterface::getInstance();
		if (!ki.Init()) {
			std::wcout << L"Kernel Interface Init() failed" << std::endl;
			goto error;
		}
		if (!ki.Handshake()) {
			std::wcout << L"Kernel Interface Handshake() failed" << std::endl;
			goto error;
		}
		if (ki.RecvWait() == SRR_TIMEOUT) {
			std::wcout << L"Ping -> ";
			if (!ki.Ping()) {
				std::wcout << L"Got no valid PONG, abort!" << std::endl;
			}
			else std::wcout << L"PONG!" << std::endl;
		}
		std::wcout << L"Driver shutdown .." << std::endl;
		ki.Exit();
	}
	catch (std::runtime_error& err) {
		std::wcout << err.what() << std::endl;
	}
error:
	Sleep(3000);
}