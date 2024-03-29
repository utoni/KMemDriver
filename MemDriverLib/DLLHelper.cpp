#include "stdafx.h"
#include "DLLHelper.h"
#include "KInterface.h"

#include <sstream>
#include <Windows.h>


static HMODULE sym_res_loadlib(IN const char * const module_name,
	IN PVOID const symbol_resolver_user_data);
static FARPROC sym_res_getproc(IN HMODULE const module_base,
	IN const char * const proc_name, IN PVOID const symbol_resolver_user_data);
static BOOL sym_res_freelib(IN HMODULE const module_base,
	IN PVOID const symbol_resolver_user_data);

const struct symbol_resolver_data sym_loadlib = symbol_resolver_data(sym_res_loadlib,
	sym_res_getproc, sym_res_freelib);

static HMODULE sym_res_loadlib(IN const char * const module_name,
	IN PVOID const symbol_resolver_user_data)
{
	UNREFERENCED_PARAMETER(symbol_resolver_user_data);

	return LoadLibraryA(module_name);
}

static FARPROC sym_res_getproc(IN HMODULE const module_base,
	IN const char * const proc_name, IN PVOID const symbol_resolver_user_data)
{
	UNREFERENCED_PARAMETER(symbol_resolver_user_data);

	return GetProcAddress(module_base, proc_name);
}

static BOOL sym_res_freelib(IN HMODULE const module_base, IN PVOID const symbol_resolver_user_data)
{
	UNREFERENCED_PARAMETER(symbol_resolver_user_data);

	return FreeLibrary(module_base);
}

SymbolResolver::SymbolResolver(struct symbol_resolver_data const * const srd, PVOID symbol_resolver_user_Data)
	: srd(srd), symbol_resolver_user_data(symbol_resolver_user_data)
{
}

SymbolResolver::~SymbolResolver()
{
}

HMODULE SymbolResolver::LoadLibrary(IN const char * const module_name)
{
	return srd->loadlib(module_name, symbol_resolver_user_data);
}

FARPROC SymbolResolver::GetProcAddress(IN HMODULE const module_base,
	IN const char * const proc_name)
{
	return srd->getproc(module_base, proc_name, symbol_resolver_user_data);
}

BOOL SymbolResolver::FreeLibrary(IN HMODULE const module_base)
{
	return srd->freelib(module_base, symbol_resolver_user_data);
}

template<SIZE_T s>
bool SymbolResolver::ResolveAllFunctionSymbols(ResolvedDllArray<s>& rda)
{
	bool result = true;

	for (auto& unresolved : rda) {
		unresolved.moduleBase = this->LoadLibrary(unresolved.baseDllName);
		if (!unresolved.moduleBase) {
			result = false;
			continue;
		}

		unresolved.resolvedProc = this->GetProcAddress(unresolved.moduleBase,
			unresolved.functionName);
		if (!unresolved.resolvedProc) {
			result = false;
		}
	}

	return result;
}

template<SIZE_T s>
bool SymbolResolver::CleanupAllFunctionSymbols(ResolvedDllArray<s>& rda)
{
	bool result = true;

	for (auto& unresolved : rda) {
		result = this->FreeLibrary(unresolved.moduleBase);
		unresolved.moduleBase = NULL;
		unresolved.resolvedProc = NULL;
	}

	return result;
}

bool SymbolResolver::LoadAndTestLibraryEntry(const char * const fullDllPath)
{
	HMODULE TestDLLModule = this->LoadLibrary(fullDllPath);
	LibEntry_FN LibEntryProc = (LibEntry_FN)this->GetProcAddress(TestDLLModule,
		"LibEntry");
	if (LibEntryProc) {
		LibEntryProc(NULL);
		return true;
	}
	else {
		return false;
	}
	this->FreeLibrary(TestDLLModule);
}

bool VerifyPeHeader(UINT8 const * const buf, SIZE_T siz, IMAGE_NT_HEADERS ** const return_NTHeader)
{
	IMAGE_DOS_HEADER *m_DOSHeader;

	if (!return_NTHeader || !buf || siz < sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_FILE_HEADER) +
		sizeof(IMAGE_OPTIONAL_HEADER64))
	{
		return false;
	}
	*return_NTHeader = NULL;
	m_DOSHeader = MakePtr(IMAGE_DOS_HEADER *, buf, 0);

	if (m_DOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return false;
	}

	*return_NTHeader = MakePtr(IMAGE_NT_HEADERS *, buf, m_DOSHeader->e_lfanew);
	if ((*return_NTHeader)->Signature != IMAGE_NT_SIGNATURE)
	{
		return false;
	}

	return true;
}

static FARPROC GetRemoteProcAddress(SymbolResolver& symres,
	HMODULE localMod, HMODULE remoteMod, char *func_name)
{
	/*
	 * Account for potential differences in base address
	 * of modules in different processes.
	 */
	ULONGLONG delta = MakeDelta(ULONGLONG, remoteMod, localMod);
	return MakePtr(FARPROC, symres.GetProcAddress(localMod, func_name), delta);
}

static HMODULE GetRemoteModuleHandle(char *module_name,
	std::vector<MODULE_DATA> &modules)
{
	SIZE_T remote_module_name_length;

	for (auto& mod : modules) {
		remote_module_name_length = strnlen(mod.BaseDllName, sizeof mod.BaseDllName);
		if (strlen(module_name) == remote_module_name_length &&
			!_strnicmp(module_name, mod.BaseDllName, remote_module_name_length))
		{
			return (HMODULE)mod.DllBase;
		}
	}

	return NULL;
}

static PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva, PIMAGE_NT_HEADERS pNTHeader)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
	unsigned int i;

	for (i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++)
	{
		// This 3 line idiocy is because Watcom's linker actually sets the
		// Misc.VirtualSize field to 0.  (!!! - Retards....!!!)
		DWORD size = section->Misc.VirtualSize;
		if (0 == size)
			size = section->SizeOfRawData;

		// Is the RVA within this section?
		if ((rva >= section->VirtualAddress) &&
			(rva < (section->VirtualAddress + size)))
			return section;
	}

	return 0;
}

static LPVOID GetPtrFromRVA(DWORD rva, IMAGE_NT_HEADERS *pNTHeader, PBYTE imageBase)
{
	PIMAGE_SECTION_HEADER pSectionHdr;
	INT delta;

	pSectionHdr = GetEnclosingSectionHeader(rva, pNTHeader);
	if (!pSectionHdr)
		return 0;

	delta = (INT)(pSectionHdr->VirtualAddress - pSectionHdr->PointerToRawData);
	return (PVOID)(imageBase + rva - delta);
}

DLLHelper::DLLHelper(SymbolResolver& symres)
	: m_symbolResolver(symres)
{
}


DLLHelper::~DLLHelper()
{
	if (m_DLLPtr) {
		delete m_DLLPtr;
	}
}

bool DLLHelper::Init(HANDLE targetPID, const char * const fullDllPath) {
	if (!targetPID) {
		return false;
	}
	m_TargetPID = targetPID;
	m_DLLPath = fullDllPath;

	HANDLE hFile = CreateFileA(m_DLLPath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		std::stringstream err_str;
		err_str << "Open file '" << m_DLLPath << "': " << GetLastError() << std::endl;
		throw std::runtime_error(err_str.str());
		return false;
	}

	if (GetFileAttributesA(m_DLLPath.c_str()) & FILE_ATTRIBUTE_COMPRESSED) {
		m_DLLSize = GetCompressedFileSizeA(m_DLLPath.c_str(), NULL);
	}
	else {
		m_DLLSize = GetFileSize(hFile, NULL);
	}

	m_DLLPtr = new UINT8[m_DLLSize];

	DWORD nBytes = 0;
	if (!ReadFile(hFile, m_DLLPtr, m_DLLSize, &nBytes, FALSE)) {
		std::stringstream err_str;
		err_str << "Read file '" << m_DLLPath << "': " << GetLastError() << std::endl;
		throw std::runtime_error(err_str.str());
		return false;
	}
	if (m_DLLSize != nBytes) {
		std::stringstream err_str;
		err_str << "Read file '" << m_DLLPath << "': returned "
			<< nBytes << " != " << m_DLLSize << std::endl;
		throw std::runtime_error(err_str.str());
		return false;
	}

	CloseHandle(hFile);
	return true;
}

bool DLLHelper::VerifyHeader()
{
	if (!m_DLLPtr) {
		return false;
	}

	return VerifyPeHeader(m_DLLPtr, m_DLLSize, &m_NTHeader);
}

bool DLLHelper::InitTargetMemory(UINT64 preferredVirtualAddress)
{
	if (!m_DLLPtr || !m_NTHeader) {
		return false;
	}

	PVOID wantedAddress = (PVOID)preferredVirtualAddress;
	SIZE_T wantedSize = m_NTHeader->OptionalHeader.SizeOfImage;
	KInterface& ki = KInterface::getInstance();
	if (!ki.VAlloc(m_TargetPID, &wantedAddress, &wantedSize, PAGE_EXECUTE_READWRITE)) {
		return false;
	}
	if (wantedSize < m_NTHeader->OptionalHeader.SizeOfImage) {
		return false;
	}

	m_TargetBaseAddress = wantedAddress;
	return true;
}

bool DLLHelper::FixImports()
{
	IMAGE_IMPORT_DESCRIPTOR *impDesc = (IMAGE_IMPORT_DESCRIPTOR *)GetPtrFromRVA(
		(DWORD)(m_NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress),
		m_NTHeader,
		(PBYTE)m_DLLPtr);
	char *module_name;
	std::vector<MODULE_DATA> modules;
	KInterface& ki = KInterface::getInstance();

	if (!m_TargetPID || !m_TargetBaseAddress || !m_NTHeader ||
		!m_NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
	{
		std::stringstream err_str;
		err_str << "FixImports pre-requirement failed [PID: " << m_TargetPID << ", BaseAddress: "
			<< m_TargetBaseAddress << ", NTHeader: " << m_NTHeader;
		if (m_NTHeader) {
			err_str << " ImportTableSize: " << m_NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		}
		err_str << "]";
		throw std::runtime_error(err_str.str());
		return false;
	}
	if (!ki.Modules(m_TargetPID, modules)) {
		return false;
	}

	//   Loop through all the required modules
	while ((module_name = (char *)GetPtrFromRVA((DWORD)(impDesc->Name), m_NTHeader,
		(PBYTE)m_DLLPtr)))
	{
		HMODULE localMod = m_symbolResolver.LoadLibrary(module_name);
		HMODULE remoteMod = GetRemoteModuleHandle(module_name, modules);

		if (!remoteMod) {
			std::stringstream err_str;
			err_str << "Module '" << module_name << "' does not exist in the target process "
				<< m_TargetPID << " and we are not allowed to load it.";
			throw std::runtime_error(err_str.str());
			return false;
		}

		IMAGE_THUNK_DATA *itd =
			(IMAGE_THUNK_DATA *)GetPtrFromRVA((DWORD)(impDesc->FirstThunk), m_NTHeader,
			(PBYTE)m_DLLPtr);

		while (itd->u1.AddressOfData)
		{
			IMAGE_IMPORT_BY_NAME *iibn;
			iibn = (IMAGE_IMPORT_BY_NAME *)GetPtrFromRVA((DWORD)(itd->u1.AddressOfData),
				m_NTHeader, (PBYTE)m_DLLPtr);

			itd->u1.Function = MakePtr(ULONGLONG, GetRemoteProcAddress(m_symbolResolver,
				localMod, remoteMod, (char *)iibn->Name), 0);

			itd++;
		}
		impDesc++;
	}

	return true;
}

bool DLLHelper::FixRelocs()
{
	unsigned long long ImageBase;
	unsigned int nBytes = 0;
	unsigned long long delta;
	IMAGE_BASE_RELOCATION *reloc;

	if (!m_TargetPID || !m_TargetBaseAddress || !m_NTHeader ||
		!m_NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
	{
		std::stringstream err_str;
		err_str << "FixRelocs pre-requirement failed [PID: " << m_TargetPID << ", BaseAddress: "
			<< m_TargetBaseAddress << ", NTHeader: " << m_NTHeader;
		if (m_NTHeader) {
			err_str << " RelocTableSize: " << m_NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
		}
		err_str << "]";
		throw std::runtime_error(err_str.str());
		return false;
	}

	reloc = (IMAGE_BASE_RELOCATION *)GetPtrFromRVA(
		(DWORD)(m_NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress),
		m_NTHeader, (PBYTE)m_DLLPtr);
	ImageBase = m_NTHeader->OptionalHeader.ImageBase;
	delta = MakeDelta(unsigned long long, m_TargetBaseAddress, ImageBase);

	while (1)
	{
		unsigned long long *locBase =
			(unsigned long long *)GetPtrFromRVA((DWORD)(reloc->VirtualAddress), m_NTHeader,
			(PBYTE)m_DLLPtr);
		unsigned int numRelocs = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);

		if (nBytes >= m_NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
			break;
		}

		unsigned short *locData = MakePtr(unsigned short *, reloc, sizeof(IMAGE_BASE_RELOCATION));
		for (unsigned int i = 0; i < numRelocs; i++)
		{
			if (((*locData >> 12) & IMAGE_REL_BASED_HIGHLOW))
				*MakePtr(unsigned long long *, locBase, (*locData & 0x0FFF)) += delta;

			locData++;
		}

		nBytes += reloc->SizeOfBlock;
		reloc = (IMAGE_BASE_RELOCATION *)locData;
	}

	return true;
}

bool DLLHelper::CopyHeaderAndSections()
{
	IMAGE_SECTION_HEADER *header;
	unsigned int nBytes = 0;
	unsigned int virtualSize = 0;
	unsigned int n = 0;
	KInterface& ki = KInterface::getInstance();

	if (!m_TargetPID || !m_TargetBaseAddress || !m_NTHeader)
	{
		std::stringstream err_str;
		err_str << "CopyHeaderAndSections pre-requirement failed [PID: " << m_TargetPID << ", BaseAddress: "
			<< m_TargetBaseAddress << ", NTHeader: " << m_NTHeader << "]";
		throw std::runtime_error(err_str.str());
		return false;
	}

	if (!ki.WPM(m_TargetPID, m_TargetBaseAddress, m_DLLPtr,
		m_NTHeader->FileHeader.SizeOfOptionalHeader +
		sizeof(m_NTHeader->FileHeader) +
		sizeof(m_NTHeader->Signature), NULL))
	{
		std::stringstream err_str;
		err_str << "CopyHeaderAndSections failed [PID: " << m_TargetPID << ", BaseAddress: "
			<< m_TargetBaseAddress << ", NTHeader: " << m_NTHeader << "]";
		throw std::runtime_error(err_str.str());
		return false;
	}

	header = IMAGE_FIRST_SECTION(m_NTHeader);
	for (unsigned int i = 0; m_NTHeader->FileHeader.NumberOfSections; i++)
	{
		if (nBytes >= m_NTHeader->OptionalHeader.SizeOfImage)
			break;

		if (!ki.WPM(m_TargetPID, MakePtr(LPVOID, m_TargetBaseAddress, header->VirtualAddress),
			MakePtr(BYTE *, m_DLLPtr, header->PointerToRawData), header->SizeOfRawData, NULL))
		{
			std::stringstream err_str;
			err_str << "CopyHeaderAndSections failed [PID: " << m_TargetPID << ", BaseAddress: "
				<< m_TargetBaseAddress << ", NTHeader: " << m_NTHeader
				<< ", Section: " << header->Name << ", VA: " << header->VirtualAddress
				<< ", Size: " << header->SizeOfRawData << "]";
			throw std::runtime_error(err_str.str());
			return false;
		}

		virtualSize = header->VirtualAddress;
		header++;
		virtualSize = header->VirtualAddress - virtualSize;
		nBytes += virtualSize;
	}

	return true;
}