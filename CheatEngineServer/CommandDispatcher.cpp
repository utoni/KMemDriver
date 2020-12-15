#include "CommandDispatcher.h"

#include <winsock.h>

#include <iostream>


static int recvall(SOCKET s, void* buf, int size, int flags)
{
	int totalreceived = 0;
	int sizeleft = size;
	char* buffer = (char*)buf;

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

static int sendall(SOCKET s, void* buf, int size, int flags)
{
	int totalsent = 0;
	int sizeleft = size;
	char* buffer = (char*)buf;

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

int DispatchCommand(CEConnection& con, char command)
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
	case CMD_OPENPROCESS: {
		int pid = 0;

		if (recvall(con.getSocket(), &pid, sizeof(pid), MSG_WAITALL) > 0)
		{
			if (sendall(con.getSocket(), &pid, sizeof(pid), 0) > 0) {
				return 0;
			}
		}
		break;
	}

	case CMD_CREATETOOLHELP32SNAPSHOT: {
		UINT32 result = 0x1;
		CeCreateToolhelp32Snapshot params;

		if (recvall(con.getSocket(), &params, sizeof(CeCreateToolhelp32Snapshot), MSG_WAITALL) > 0)
		{
#if 0
			std::cout << "Calling CreateToolhelp32Snapshot with flags 0x" << std::hex << params.dwFlags
				<< " for PID 0x" << std::hex << params.th32ProcessID << std::endl;
#endif
			if (sendall(con.getSocket(), &result, sizeof(result), 0) > 0)
			{
				return 0;
			}
		}
		break;
	}

	case CMD_PROCESS32FIRST:
		con.m_cachedProcesses.clear();
		KInterface::getInstance().MtProcesses(con.m_cachedProcesses);
	case CMD_PROCESS32NEXT: {
		UINT32 toolhelpsnapshot;

		if (recvall(con.getSocket(), &toolhelpsnapshot, sizeof(toolhelpsnapshot), MSG_WAITALL) > 0)
		{
			if (con.m_cachedProcesses.size() > 0) {
				PROCESS_DATA pd = con.m_cachedProcesses[0];
				int imageNameLen = (int)strnlen(pd.ImageName, sizeof(pd.ImageName));
				CeProcessEntry* pcpe = (CeProcessEntry*)malloc(sizeof(*pcpe) + imageNameLen);

				con.m_cachedProcesses.erase(con.m_cachedProcesses.begin());
				if (pcpe == NULL) {
					return 1;
				}
				pcpe->pid = (int)((ULONG_PTR)pd.UniqueProcessId);
				pcpe->processnamesize = imageNameLen;
				memcpy(((BYTE*)pcpe) + sizeof(*pcpe), pd.ImageName, imageNameLen);
				pcpe->result = 1;
				if (sendall(con.getSocket(), pcpe, sizeof(*pcpe) + imageNameLen, 0) > 0)
				{
					free(pcpe);
					return 0;
				}
				free(pcpe);
			}
			else {
				CeProcessEntry cpe;
				cpe.pid = 0;
				cpe.processnamesize = 0;
				cpe.result = 0;
				if (sendall(con.getSocket(), &cpe, sizeof(cpe), 0) > 0)
				{
					return 0;
				}
			}
		}
		break;
	}

	case CMD_CLOSEHANDLE: {
		UINT32 handle;
		if (recvall(con.getSocket(), &handle, sizeof(handle), MSG_WAITALL) > 0)
		{
			UINT32 r = 1;
			sendall(con.getSocket(), &r, sizeof(r), 0);
			return 0;
		}
		break;
	}

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
	case CMD_GETARCHITECTURE: {
		unsigned char arch;
#ifdef __i386__
		arch = 0;
#endif
#ifdef __x86_64__
		arch = 1;
#endif
#ifdef __arm__
		arch = 2;
#endif
#ifdef __aarch64__
		arch = 3;
#endif
		sendall(con.getSocket(), &arch, sizeof(arch), 0);
		return 0;
	}

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

int CheckForAndDispatchCommand(CEConnection& con)
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