#pragma once

#include <string>
#include <Windows.h>


#define MakePtr(cast, ptr, addValue) (cast)((DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))
#define MakeDelta(cast, x, y) (cast) ((DWORD_PTR)(x) - (DWORD_PTR)(y))

struct ResolvedDllEntry {
	const char * const baseDllName;
	const char * const functionName;

	HMODULE moduleBase;
	FARPROC resolvedProc;
};
template<SIZE_T s>
using ResolvedDllArray = std::array<struct ResolvedDllEntry, s>;

typedef HMODULE(*load_library_cb)(IN const char * const module_name,
	IN PVOID const symbol_resolver_user_data);
typedef FARPROC(*get_proc_address_cb)(IN HMODULE const module_base,
	IN const char * const proc_name, IN PVOID const symbol_resolver_user_data);
typedef BOOL(*free_library_cb)(IN HMODULE const module_base,
	IN PVOID const symbol_resolver_user_data);

struct symbol_resolver_data {
	explicit symbol_resolver_data(load_library_cb _loadlib, get_proc_address_cb _getproc, free_library_cb _freelib)
		: loadlib(_loadlib), getproc(_getproc), freelib(_freelib) {}
	load_library_cb loadlib;
	get_proc_address_cb getproc;
	free_library_cb freelib;
};

extern const struct symbol_resolver_data sym_loadlib;

typedef void(*LibEntry_FN)(PVOID user_ptr);

bool VerifyPeHeader(UINT8 const * const buf, SIZE_T siz, IMAGE_NT_HEADERS ** const return_NTHeader);

class SymbolResolver
{
public:
	explicit SymbolResolver(struct symbol_resolver_data const * const srd = &sym_loadlib,
		PVOID const symbol_resolver_user_data = NULL);
	~SymbolResolver();

	HMODULE LoadLibrary(IN const char * const module_name);
	FARPROC GetProcAddress(IN HMODULE const module_base,
		IN const char * const proc_name);
	BOOL FreeLibrary(IN HMODULE const module_base);

	template<SIZE_T s>
	bool ResolveAllFunctionSymbols(ResolvedDllArray<s>& rda);
	template<SIZE_T s>
	bool CleanupAllFunctionSymbols(ResolvedDllArray<s>& rda);
	bool LoadAndTestLibraryEntry(const char * const fullDllPath);
private:
	struct symbol_resolver_data const * const srd;
	PVOID symbol_resolver_user_data;
};

class DLLHelper
{
public:
	DLLHelper(SymbolResolver& symres);
	~DLLHelper();

	bool Init(HANDLE targetPID, const char * const fullDllPath);
	bool VerifyHeader();
	bool InitTargetMemory(UINT64 preferredVirtualAddress = 0x0);
	bool HasImports() {
		return m_NTHeader &&
			m_NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
	}
	bool FixImports();
	bool HasRelocs() {
		return m_NTHeader &&
			m_NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
	}
	bool FixRelocs();
	bool CopyHeaderAndSections();
	UINT64 GetEntryPoint() {
		if (!m_NTHeader) {
			return 0;
		}
		return (UINT64)m_TargetBaseAddress + m_NTHeader->OptionalHeader.AddressOfEntryPoint;
	}
	UINT64 GetBaseAddress() {
		return (UINT64)m_TargetBaseAddress;
	}
	UINT64 GetDllProcAddress(const char * const proc_name) {
		HMODULE hBase = m_symbolResolver.LoadLibrary(m_DLLPath.c_str());
		FARPROC hEntry = m_symbolResolver.GetProcAddress(hBase, proc_name);
		UINT64 result = ((UINT64)hEntry - (UINT64)hBase) + (UINT64)m_TargetBaseAddress;
		m_symbolResolver.FreeLibrary(hBase);
		return result;
	}

private:
	SymbolResolver& m_symbolResolver;

	HANDLE m_TargetPID = 0;
	std::string m_DLLPath;
	DWORD m_DLLSize = 0;
	UINT8 *m_DLLPtr = nullptr;
	IMAGE_NT_HEADERS *m_NTHeader = nullptr;
	PVOID m_TargetBaseAddress = nullptr;
};

