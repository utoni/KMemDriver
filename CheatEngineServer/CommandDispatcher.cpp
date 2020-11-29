#include "CommandDispatcher.h"

#include <winsock.h>

#include <iostream>


static int recvall(SOCKET s, void *buf, int size, int flags)
{
	int totalreceived = 0;
	int sizeleft = size;
	char *buffer = (char*)buf;

	flags = flags | MSG_WAITALL;
	while (sizeleft > 0)
	{
		int i = recv(s, &buffer[totalreceived], sizeleft, flags);
		if (i == 0)
		{
			std::cout << "recv returned 0" << std::endl;
			return i;
		}
		if (i <= -1)
		{
			std::cout << "recv returned -1" << std::endl;
			if (errno == EINTR)
			{
				std::cout << "errno = EINTR\n" << std::endl;
				i = 0;
			}
			else
			{
				std::cout << "Error during recvall: " << (int)i << ". errno=" << errno << "\n" << std::endl;
				return i; //read error, or disconnected
			}
		}
		totalreceived += i;
		sizeleft -= i;
	}
	return totalreceived;
}

static int sendall(SOCKET s, void *buf, int size, int flags)
{
	int totalsent = 0;
	int sizeleft = size;
	char *buffer = (char*)buf;

	while (sizeleft > 0)
	{
		int i = send(s, &buffer[totalsent], sizeleft, flags);

		if (i == 0)
		{
			return i;
		}
		if (i == -1)
		{
			if (errno == EINTR)
				i = 0;
			else
			{
				std::cout << "Error during sendall: " << (int)i << ". errno=" << errno << "\n" << std::endl;
				return i;
			}
		}

		totalsent += i;
		sizeleft -= i;
	}

	return totalsent;
}

int DispatchCommand(CEConnection & con, char command)
{
	enum ce_command cmd = (enum ce_command)command;

	std::cout << "Command: " << ce_command_to_string(cmd) << std::endl;

	switch (cmd)
	{
	case CMD_GETVERSION:
		break;
	case CMD_CLOSECONNECTION:
		break;
	case CMD_TERMINATESERVER:
		break;
	case CMD_OPENPROCESS:
		break;
	case CMD_CREATETOOLHELP32SNAPSHOT: {
		HANDLE result = (HANDLE)((ULONG_PTR)0x1);
		CeCreateToolhelp32Snapshot params;

		if (recvall(con.getSocket(), &params, sizeof(CeCreateToolhelp32Snapshot), MSG_WAITALL) > 0)
		{
			std::cout << "Calling CreateToolhelp32Snapshot with flags 0x" << std::hex << params.dwFlags
				<< " for PID 0x" << std::hex << params.th32ProcessID << std::endl;
		}
		if (sendall(con.getSocket(), &result, sizeof(result), 0) == sizeof(result))
		{
			return 0;
		}
		break;
	}
	case CMD_PROCESS32FIRST:
		break;
	case CMD_PROCESS32NEXT:
		break;
	case CMD_CLOSEHANDLE:
		break;
	case CMD_VIRTUALQUERYEX:
		break;
	case CMD_READPROCESSMEMORY:
		break;
	case CMD_WRITEPROCESSMEMORY:
		break;
	case CMD_STARTDEBUG:
		break;
	case CMD_STOPDEBUG:
		break;
	case CMD_WAITFORDEBUGEVENT:
		break;
	case CMD_CONTINUEFROMDEBUGEVENT:
		break;
	case CMD_SETBREAKPOINT:
		break;
	case CMD_REMOVEBREAKPOINT:
		break;
	case CMD_SUSPENDTHREAD:
		break;
	case CMD_RESUMETHREAD:
		break;
	case CMD_GETTHREADCONTEXT:
		break;
	case CMD_SETTHREADCONTEXT:
		break;
	case CMD_GETARCHITECTURE:
		break;
	case CMD_MODULE32FIRST:
		break;
	case CMD_MODULE32NEXT:
		break;
	case CMD_GETSYMBOLLISTFROMFILE:
		break;
	case CMD_LOADEXTENSION:
		break;
	case CMD_ALLOC:
		break;
	case CMD_FREE:
		break;
	case CMD_CREATETHREAD:
		break;
	case CMD_LOADMODULE:
		break;
	case CMD_SPEEDHACK_SETSPEED:
		break;
	case CMD_VIRTUALQUERYEXFULL:
		break;
	case CMD_GETREGIONINFO:
		break;
	case CMD_AOBSCAN:
		break;
	case CMD_COMMANDLIST2:
		break;
	}

	return 1;
}

int CheckForAndDispatchCommand(CEConnection & con)
{
	int r;
	char command;

	r = recv(con.getSocket(), &command, 1, 0);
	if (r == 1)
	{
		return DispatchCommand(con, command);
	}

	return 0;
}