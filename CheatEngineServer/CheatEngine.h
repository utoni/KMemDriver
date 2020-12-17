#pragma once

#include "KInterface.h"
#include <winsock.h>

#include <vector>

#define CE_PORT "52736"
#define MSG_WAITALL 0x8

#pragma warning(push)
#pragma warning(disable : 26812)
typedef enum ce_command {
	CMD_GETVERSION = 0,
	CMD_CLOSECONNECTION,
	CMD_TERMINATESERVER,
	CMD_OPENPROCESS,
	CMD_CREATETOOLHELP32SNAPSHOT,
	CMD_PROCESS32FIRST,
	CMD_PROCESS32NEXT,
	CMD_CLOSEHANDLE,
	CMD_VIRTUALQUERYEX,
	CMD_READPROCESSMEMORY,
	CMD_WRITEPROCESSMEMORY,
	CMD_STARTDEBUG,
	CMD_STOPDEBUG,
	CMD_WAITFORDEBUGEVENT,
	CMD_CONTINUEFROMDEBUGEVENT,
	CMD_SETBREAKPOINT,
	CMD_REMOVEBREAKPOINT,
	CMD_SUSPENDTHREAD,
	CMD_RESUMETHREAD,
	CMD_GETTHREADCONTEXT,
	CMD_SETTHREADCONTEXT,
	CMD_GETARCHITECTURE,
	CMD_MODULE32FIRST,
	CMD_MODULE32NEXT,
	CMD_GETSYMBOLLISTFROMFILE,
	CMD_LOADEXTENSION,
	CMD_ALLOC,
	CMD_FREE,
	CMD_CREATETHREAD,
	CMD_LOADMODULE,
	CMD_SPEEDHACK_SETSPEED,
	CMD_VIRTUALQUERYEXFULL,
	CMD_GETREGIONINFO,
	CMD_AOBSCAN = 200,
	CMD_COMMANDLIST2 = 255,

	CMD_MAX
} ce_command;

static inline char const* ce_command_to_string(enum ce_command cmd)
{
	static char const* const cmd_map[] = {
	"CMD_GETVERSION", "CMD_CLOSECONNECTION", "CMD_TERMINATESERVER", "CMD_OPENPROCESS",
	"CMD_CREATETOOLHELP32SNAPSHOT", "CMD_PROCESS32FIRST", "CMD_PROCESS32NEXT", "CMD_CLOSEHANDLE",
	"CMD_VIRTUALQUERYEX", "CMD_READPROCESSMEMORY", "CMD_WRITEPROCESSMEMORY", "CMD_STARTDEBUG",
	"CMD_STOPDEBUG", "CMD_WAITFORDEBUGEVENT", "CMD_CONTINUEFROMDEBUGEVENT", "CMD_SETBREAKPOINT",
	"CMD_REMOVEBREAKPOINT", "CMD_SUSPENDTHREAD", "CMD_RESUMETHREAD", "CMD_GETTHREADCONTEXT",
	"CMD_SETTHREADCONTEXT", "CMD_GETARCHITECTURE", "CMD_MODULE32FIRST", "CMD_MODULE32NEXT",
	"CMD_GETSYMBOLLISTFROMFILE", "CMD_LOADEXTENSION", "CMD_ALLOC", "CMD_FREE", "CMD_CREATETHREAD",
	"CMD_LOADMODULE", "CMD_SPEEDHACK_SETSPEED", "CMD_VIRTUALQUERYEXFULL", "CMD_GETREGIONINFO",
	"CMD_AOBSCAN", "CMD_COMMANDLIST2"
	};
	if (cmd < 0 || cmd >= CMD_MAX)
	{
		return "Unknown Command";
	}
	return cmd_map[cmd];
}
#pragma warning(pop)

#pragma pack(1)
typedef struct {
	DWORD dwFlags;
	DWORD th32ProcessID;
} CeCreateToolhelp32Snapshot, * PCeCreateToolhelp32Snapshot;

typedef struct {
	int result;
	int pid;
	int processnamesize;
	//processname
} CeProcessEntry, * PCeProcessEntry;

typedef struct {
	int result;
	int64_t modulebase;
	int modulesize;
	int modulenamesize;
	//modulename
} CeModuleEntry, * PCeModuleEntry;

typedef struct {
	uint32_t handle;
	uint64_t address;
	uint32_t size;
	uint8_t  compress;
} CeReadProcessMemoryInput, * PCeReadProcessMemoryInput;

typedef struct {
	int read;
} CeReadProcessMemoryOutput, * PCeReadProcessMemoryOutput;

typedef struct {
	int32_t handle;
	int64_t address;
	int32_t size;
} CeWriteProcessMemoryInput, * PCeWriteProcessMemoryInput;

typedef struct {
	int32_t written;
} CeWriteProcessMemoryOutput, * PCeWriteProcessMemoryOutput;
#pragma pack()

class CEConnection {
public:
	explicit CEConnection(SOCKET s) : m_sock(s) {}
	SOCKET getSocket(void) { return m_sock; }
	void closeSocket(void) { closesocket(m_sock); }

	std::vector<PROCESS_DATA> m_cachedProcesses;
	std::vector<MODULE_DATA> m_cachedModules;
private:
	SOCKET m_sock;
};