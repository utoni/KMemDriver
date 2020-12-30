#include "CommandDispatcher.h"

#include <winsock.h>

#include <iostream>

#define SPECIAL_TOOLHELP_SNAPSHOT_PROCESS 0x02
#define SPECIAL_TOOLHELP_SNAPSHOT_PROCESS_HANDLE 0x01;

static char const versionstring[] = "CHEATENGINE Network 2.0";

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
			std::wcout << "recv returned 0" << std::endl;
			return i;
		}
		if (i <= -1)
		{
			std::wcout << "recv returned -1" << std::endl;
			if (errno == EINTR)
			{
				std::wcout << "errno = EINTR\n" << std::endl;
				i = 0;
			}
			else
			{
				std::wcout << "Error during recvall: " << (int)i << ". errno=" << errno << "\n" << std::endl;
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
				std::wcout << "Error during sendall: " << (int)i << ". errno=" << errno << "\n" << std::endl;
				return i;
			}
		}

		totalsent += i;
		sizeleft -= i;
	}

	return totalsent;
}

CommandReturn DispatchCommand(CEConnection& con, char command)
{
	CommandReturn cret = CommandReturn::CR_FAIL_UNHANDLED;
	enum ce_command cmd = (enum ce_command)command;

	//std::wcout << "Command: " << ce_command_to_string(cmd) << std::endl;

	switch (cmd)
	{
	case CMD_GETVERSION: {
		PCeVersion v;
		int versionsize = (int)strlen(versionstring);
		v = (PCeVersion)malloc(sizeof(CeVersion) + versionsize);
		if (v == NULL) {
			cret = CommandReturn::CR_FAIL_ALLOC;
			break;
		}
		v->stringsize = versionsize;
		v->version = 1;
		memcpy((char*)v + sizeof(CeVersion), versionstring, versionsize);
		if (sendall(con.getSocket(), v, sizeof(CeVersion) + versionsize, 0) > 0) {
			cret = CommandReturn::CR_OK;
		}
		else {
			cret = CommandReturn::CR_FAIL_NETWORK;
			free(v);
		}
	}

	case CMD_CLOSECONNECTION:
		break;
	case CMD_TERMINATESERVER:
		break;

	case CMD_OPENPROCESS: {
		int pid = 0;

		if (recvall(con.getSocket(), &pid, sizeof(pid), MSG_WAITALL) > 0)
		{
			//std::wcout << "OpenProcess for PID " << (HANDLE)pid << std::endl;
			if (sendall(con.getSocket(), &pid, sizeof(pid), 0) > 0) {
				cret = CommandReturn::CR_OK;
			}
			else {
				cret = CommandReturn::CR_FAIL_NETWORK;
			}
		}
		else {
			cret = CommandReturn::CR_FAIL_NETWORK;
		}
		break;
	}

	case CMD_CREATETOOLHELP32SNAPSHOT: {
		UINT32 result;
		CeCreateToolhelp32Snapshot params;

		if (recvall(con.getSocket(), &params, sizeof(CeCreateToolhelp32Snapshot), MSG_WAITALL) > 0)
		{
			//std::wcout << "Calling CreateToolhelp32Snapshot with flags 0x" << std::hex << params.dwFlags
				//<< " for PID 0x" << std::hex << params.th32ProcessID << std::endl;
			if (params.dwFlags == SPECIAL_TOOLHELP_SNAPSHOT_PROCESS) {
				result = SPECIAL_TOOLHELP_SNAPSHOT_PROCESS_HANDLE;
			}
			else {
				result = params.th32ProcessID;
			}
			if (sendall(con.getSocket(), &result, sizeof(result), 0) > 0)
			{
				cret = CommandReturn::CR_OK;
			}
			else {
				cret = CommandReturn::CR_FAIL_NETWORK;
			}
		}
		else {
			cret = CommandReturn::CR_FAIL_NETWORK;
		}
		break;
	}

	case CMD_PROCESS32FIRST:
		con.m_cachedProcesses.clear();
		if (KInterface::getInstance().MtProcesses(con.m_cachedProcesses) != true) {
			return CommandReturn::CR_FAIL_KMEM;
		}
	case CMD_PROCESS32NEXT: {
		UINT32 toolhelpsnapshot;

		if (recvall(con.getSocket(), &toolhelpsnapshot, sizeof(toolhelpsnapshot), MSG_WAITALL) > 0)
		{
			if (con.m_cachedProcesses.size() > 0) {
				PROCESS_DATA pd = con.m_cachedProcesses[0];
				int imageNameLen = (int)strnlen(pd.ImageName, sizeof(pd.ImageName));
				CeProcessEntry* pcpe = (CeProcessEntry*)malloc(sizeof(*pcpe) + imageNameLen);

				if (pcpe == NULL) {
					cret = CommandReturn::CR_FAIL_ALLOC;
					break;
				}
				con.m_cachedProcesses.erase(con.m_cachedProcesses.begin());
				pcpe->pid = (int)((ULONG_PTR)pd.UniqueProcessId);
				pcpe->processnamesize = imageNameLen;
				pcpe->result = 1;
				memcpy(((BYTE*)pcpe) + sizeof(*pcpe), pd.ImageName, imageNameLen);
				if (sendall(con.getSocket(), pcpe, sizeof(*pcpe) + imageNameLen, 0) > 0)
				{
					cret = CommandReturn::CR_OK;
				}
				else {
					cret = CommandReturn::CR_FAIL_NETWORK;
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
					cret = CommandReturn::CR_OK;
				}
				else {
					cret = CommandReturn::CR_FAIL_NETWORK;
				}
			}
		}
		else {
			cret = CommandReturn::CR_FAIL_NETWORK;
		}
		break;
	}

	case CMD_CLOSEHANDLE: {
		UINT32 handle;
		if (recvall(con.getSocket(), &handle, sizeof(handle), MSG_WAITALL) > 0)
		{
			UINT32 r = 1;
			if (sendall(con.getSocket(), &r, sizeof(r), 0) > 0) {
				cret = CommandReturn::CR_OK;
			}
			else {
				cret = CommandReturn::CR_FAIL_NETWORK;
			}
		}
		else {
			cret = CommandReturn::CR_FAIL_NETWORK;
		}
		break;
	}

	case CMD_READPROCESSMEMORY: {
		CeReadProcessMemoryInput params;
		PCeReadProcessMemoryOutput out;
		KERNEL_READ_REQUEST krr;

		if (recvall(con.getSocket(), &params, sizeof(params), MSG_WAITALL) > 0) {
			if (params.compress != 0) {
				cret = CommandReturn::CR_FAIL_OTHER;
				break;
			}
			out = (PCeReadProcessMemoryOutput)malloc(sizeof(*out) + params.size);
			if (out == NULL) {
				cret = CommandReturn::CR_FAIL_ALLOC;
				break;
			}
			if (KInterface::getInstance().MtRPM((HANDLE)((ULONG_PTR)params.handle), (PVOID)params.address, (BYTE*)out + sizeof(*out), params.size, &krr) != true) {
				free(out);
				cret = CommandReturn::CR_FAIL_KMEM;
				break;
			}
			if (params.size != krr.SizeReq || params.size != krr.SizeRes || krr.StatusRes != 0) {
				free(out);
				cret = CommandReturn::CR_FAIL_OTHER;
				break;
			}
			if (sendall(con.getSocket(), out, sizeof(*out) + params.size, 0) > 0)
			{
				free(out);
				cret = CommandReturn::CR_OK;
				break;
			}
			else {
				free(out);
				cret = CommandReturn::CR_FAIL_NETWORK;
			}
		}
		else {
			cret = CommandReturn::CR_FAIL_NETWORK;
		}
		break;
	}

	case CMD_WRITEPROCESSMEMORY: {
		break;
	}

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
		if (sendall(con.getSocket(), &arch, sizeof(arch), 0) > 0) {
			cret = CommandReturn::CR_OK;
		}
		else {
			cret = CommandReturn::CR_FAIL_NETWORK;
		}
		break;
	}

	case CMD_MODULE32FIRST:
	case CMD_MODULE32NEXT: {
		UINT32 toolhelpsnapshot;
		if (recvall(con.getSocket(), &toolhelpsnapshot, sizeof(toolhelpsnapshot), MSG_WAITALL) > 0)
		{
			if (cmd == CMD_MODULE32FIRST) {
				con.m_cachedModules.clear();
				//std::wcout << "Modules for PID 0x" << std::hex << toolhelpsnapshot << std::endl;
				if (KInterface::getInstance().MtModules((HANDLE)((ULONG_PTR)toolhelpsnapshot), con.m_cachedModules) != true) {
					cret = CommandReturn::CR_FAIL_KMEM;
					break;
				}
			}
			else {
				//std::wcout << "Modules NEXT for PID 0x" << std::hex << toolhelpsnapshot << std::endl;
			}
			if (con.m_cachedModules.size() > 0) {
				MODULE_DATA md = con.m_cachedModules[0];
				int imageNameLen = (int)strnlen(md.BaseDllName, sizeof(md.BaseDllName));
				CeModuleEntry* pcme = (CeModuleEntry*)malloc(sizeof(*pcme) + imageNameLen);

				if (pcme == NULL) {
					cret = CommandReturn::CR_FAIL_ALLOC;
					break;
				}
				con.m_cachedModules.erase(con.m_cachedModules.begin());
				pcme->modulebase = (INT64)md.DllBase;
				pcme->modulesize = md.SizeOfImage;
				pcme->modulenamesize = imageNameLen;
				pcme->result = 1;
				memcpy(((BYTE*)pcme) + sizeof(*pcme), md.BaseDllName, imageNameLen);
				if (sendall(con.getSocket(), pcme, sizeof(*pcme) + imageNameLen, 0) > 0)
				{
					cret = CommandReturn::CR_OK;
				}
				else {
					cret = CommandReturn::CR_FAIL_NETWORK;
				}
				free(pcme);
			}
			else {
				CeModuleEntry cme;
				cme.modulebase = 0;
				cme.modulesize = 0;
				cme.modulenamesize = 0;
				cme.result = 0;
				if (sendall(con.getSocket(), &cme, sizeof(cme), 0) > 0)
				{
					cret = CommandReturn::CR_OK;
				}
				else {
					cret = CommandReturn::CR_FAIL_NETWORK;
				}
			}
		}
		break;
	}

	case CMD_GETSYMBOLLISTFROMFILE: {
		UINT32 symbolpathsize;
		if (recvall(con.getSocket(), &symbolpathsize, sizeof(symbolpathsize), MSG_WAITALL) > 0)
		{
			char* symbolpath = (char*)malloc((SIZE_T)symbolpathsize + 1);
			symbolpath[symbolpathsize] = '\0';
			if (recvall(con.getSocket(), symbolpath, symbolpathsize, MSG_WAITALL) > 0)
			{
				//std::wcout << "Symbolpath: " << symbolpath << std::endl;
				UINT64 fail = 0;
				if (sendall(con.getSocket(), &fail, sizeof(fail), 0) > 0)
				{
					cret = CommandReturn::CR_OK;
				}
				else {
					cret = CommandReturn::CR_FAIL_NETWORK;
				}
			}
			else {
				cret = CommandReturn::CR_FAIL_NETWORK;
			}
		}
		break;
	}

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

	case CMD_VIRTUALQUERYEXFULL: {
		CeVirtualQueryExFullInput params;
		if (recvall(con.getSocket(), &params, sizeof(params), MSG_WAITALL) > 0) {
			con.m_cachedPages.clear();
			if (KInterface::getInstance().MtPages((HANDLE)((ULONG_PTR)params.handle), con.m_cachedPages) != true) {
				cret = CommandReturn::CR_FAIL_KMEM;
				break;
			}
			UINT32 count = (UINT32)con.m_cachedPages.size();
			if (sendall(con.getSocket(), &count, sizeof(count), 0) <= 0)
			{
				cret = CommandReturn::CR_FAIL_NETWORK;
				break;
			}
			cret = CommandReturn::CR_OK;
			for (auto& page : con.m_cachedPages) {
				RegionInfo out;
				out.baseaddress = (UINT64)page.BaseAddress;
				out.protection = page.Protect;
				out.size = page.RegionSize;
				out.type = page.Type;
				if (sendall(con.getSocket(), &out, sizeof(out), 0) <= 0)
				{
					cret = CommandReturn::CR_FAIL_NETWORK;
				}
			}
		}
		else {
			cret = CommandReturn::CR_FAIL_NETWORK;
		}
		break;
	}

	case CMD_VIRTUALQUERYEX:
	case CMD_GETREGIONINFO: {
		CeVirtualQueryExInput params;
		if (recvall(con.getSocket(), &params, sizeof(params), MSG_WAITALL) > 0) {
			con.m_cachedPages.clear();
			if (KInterface::getInstance().MtPages((HANDLE)((ULONG_PTR)params.handle), con.m_cachedPages, (PVOID)params.baseaddress) != true ||
				con.m_cachedPages.size() == 0)
			{
				cret = CommandReturn::CR_FAIL_KMEM;
				break;
			}
			CeVirtualQueryExOutput out;
			out.baseaddress = (UINT64)con.m_cachedPages[0].BaseAddress;
			out.protection = con.m_cachedPages[0].Protect;
			out.size = con.m_cachedPages[0].RegionSize;
			out.type = con.m_cachedPages[0].Type;
			out.result = 1;
			if (sendall(con.getSocket(), &out, sizeof(out), 0) > 0) {
				if (cmd == CMD_GETREGIONINFO) {
					uint8_t size = 0;
					if (sendall(con.getSocket(), &size, sizeof(size), 0) > 0) {
						cret = CommandReturn::CR_OK;
					}
					else {
						cret = CommandReturn::CR_FAIL_NETWORK;
					}
				}
			}
			else {
				cret = CommandReturn::CR_FAIL_NETWORK;
			}
		}
		break;
	}

	case CMD_AOBSCAN:
		break;
	case CMD_COMMANDLIST2:
		break;
	}

	return cret;
}

int CheckForAndDispatchCommand(CEConnection& con)
{
	int r;
	char command;

	r = recv(con.getSocket(), &command, 1, 0);
	if (r == 1)
	{
		enum ce_command cmd = (enum ce_command)command;

		switch (DispatchCommand(con, cmd))
		{
		case CommandReturn::CR_FAIL_UNHANDLED:
			std::wcout << "Unhandled command: " << ce_command_to_string(cmd) << std::endl;
			return 1;
		case CommandReturn::CR_FAIL_OTHER:
			std::wcout << "Unknown error for command: " << ce_command_to_string(cmd) << std::endl;
			return 1;
		case CommandReturn::CR_FAIL_NETWORK:
			std::wcout << "Network error for command: " << ce_command_to_string(cmd) << std::endl;
			return 1;
		case CommandReturn::CR_FAIL_KMEM:
			std::wcout << "Internal KMemDriver API error for command: " << ce_command_to_string(cmd) << std::endl;
			return 1;
		case CommandReturn::CR_FAIL_ALLOC:
			std::wcout << "Memory allocation failed for command: " << ce_command_to_string(cmd) << std::endl;
			return 1;
		case CommandReturn::CR_OK:
			break;
		}
	}
	else {
		std::wcout << "Command recv returned: " << r << std::endl;
		return 1;
	}

	return 0;
}