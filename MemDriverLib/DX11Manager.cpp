#include "stdafx.h"
#include "DX11Manager.h"

#include <Windows.h>

#pragma comment (lib, "D3D11.lib")

struct DDataIntern {
	IDXGISwapChain * SwapChain;
	ID3D11Device * Device;
	ID3D11DeviceContext * DeviceContext;
};

static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case(WM_DESTROY):
		PostQuitMessage(0);
		return 0;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
}

static HRESULT InitD3D(struct DDataIntern * const data, HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;

	ZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = hWnd;
	SwapChainDesc.SampleDesc.Count = 4;
	SwapChainDesc.Windowed = true;

	return D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &SwapChainDesc,
		&data->SwapChain, &data->Device, NULL, &data->DeviceContext);
}

static void CleanD3D(struct DDataIntern * const data)
{
	data->SwapChain->Release();
	data->Device->Release();
	data->DeviceContext->Release();
}

bool WINAPI GetDirectxData(struct DxData * const data)
{
	HINSTANCE hInstance = (HINSTANCE)((LONG_PTR)GetWindowLongW(GetActiveWindow(), -6));
	HWND hWnd;
	WNDCLASSEX wc;
	struct DDataIntern data_intern;

	ZeroMemory(&data_intern, sizeof(data_intern));
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WinProc;
	wc.lpszClassName = L"DxData";
	wc.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&wc);

	RECT rect = { 0, 0, 600, 400 };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	hWnd = CreateWindowEx(NULL, L"DxData", L"DxData", WS_OVERLAPPEDWINDOW, 300, 300, rect.right - rect.left,
		rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
	if (!hWnd) {
		return false;
	}

	ShowWindow(hWnd, NULL);
	data->CreateSwapChainReturn = InitD3D(&data_intern, hWnd);

	{
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0) &&
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	memcpy(data->buf, (*(UINT64 ***)data_intern.SwapChain)[8], sizeof data->buf);

	HMODULE d3d11_base = LoadLibrary(L"d3d11.dll");
	data->DeviceVTableOffset = *(UINT64 *)data_intern.Device;
	data->DeviceContextVTableOffset = *(UINT64 *)data_intern.DeviceContext;
	data->SwapChainVTableOffset = (*(UINT64 **)data_intern.SwapChain)[8] - (UINT64)d3d11_base;

	CleanD3D(&data_intern);
	CloseWindow(hWnd);
	DestroyWindow(hWnd);

	{
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0) &&
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	FreeLibrary(d3d11_base);

	return true;
}