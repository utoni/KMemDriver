#pragma once

#include "KMemDriver.h"

#include <string>
#include <vector>


typedef bool(*map_file_cb)(SymbolResolver& symres, IN MODULE_DATA&, OUT PVOID * const,
	OUT SIZE_T * const, IN PVOID const);
typedef bool(*map_file_cleanup_cb)(SymbolResolver& symres, IN MODULE_DATA&,
	IN PVOID, IN PVOID const);

struct map_file_data {
	explicit map_file_data(map_file_cb _mapfile, map_file_cleanup_cb _mapcleanup, bool _in_memory_module)
		: mapfile(_mapfile), mapcleanup(_mapcleanup), in_memory_module(_in_memory_module) {}
	map_file_cb mapfile;
	map_file_cleanup_cb mapcleanup;
	bool in_memory_module;
};

struct loadlib_user_data {
	std::vector<std::string> additionalDllSearchDirectories;
};

extern const struct map_file_data map_loadlib;
extern const struct map_file_data map_kmem;

class PatternScanner
{
public:
	explicit PatternScanner(SymbolResolver& symres,
		struct map_file_data const * const mfd = &map_loadlib,
		PVOID const map_file_user_data = NULL);
	~PatternScanner();
	void SetScanLowAddress(UINT64 startAddress) {
		m_LowAddress = startAddress;
	}
	void SetScanAddress(UINT64 startAddress) {
		m_LowAddress = startAddress;
	}
	bool Scan(MODULE_DATA& module, const char * const pattern);
private:
	bool checkPattern(MODULE_DATA& module, const char * const pattern, std::string& result);
	bool doScan(std::string& pattern, UINT8 *buf, SIZE_T size, std::vector<UINT64>& foundOffsets);

	SymbolResolver& m_symbolResolver;

	struct map_file_data const * const mfd;
	PVOID const map_file_user_data;

	UINT64 m_LowAddress = 0x0;
	UINT64 m_HighAddress = ((UINT64)-1);
};

