#pragma once

#include <D3D11.h>

struct DxData {
	HRESULT CreateSwapChainReturn;
	/* ALL Offsets are relative to the base address of d3d11.dll */
	UINT64 DeviceVTableOffset;
	UINT64 DeviceContextVTableOffset;
	UINT64 SwapChainVTableOffset;
	BYTE buf[64];
};

bool WINAPI GetDirectxData(struct DxData * const data);