#include "CommandDispatcher.h"

#include <winsock.h>

#include <iostream>

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
	case CMD_CREATETOOLHELP32SNAPSHOT:
		break;
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