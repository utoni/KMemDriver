#pragma once

#include "KMemDriver.h"

#include <string>
#include <vector>


typedef bool(*map_file_cb)(IN MODULE_DATA&, OUT PVOID * const,
	OUT SIZE_T * const, IN PVOID const);
typedef bool(*map_file_cleanup_cb)(IN MODULE_DATA&,
	IN PVOID, IN PVOID const);

struct map_file_data {
	map_file_cb map_file;
	map_file_cleanup_cb map_file_cleanup;
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
	explicit PatternScanner(struct map_file_data const * const mfd = &map_loadlib, PVOID map_file_user_data = NULL);
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
	bool doScan(UINT8 *buf, SIZE_T size, std::vector<UINT64>& foundOffsets);

	struct map_file_data const * const mfd;
	UINT64 m_LowAddress = 0x0;
	UINT64 m_HighAddress = ((UINT64)-1);
	PVOID map_file_user_data;
};

