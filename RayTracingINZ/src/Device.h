#pragma once


#include <Windows.h>
#include <d3d11_4.h>
#include <wrl/client.h>
#include <dxgi1_6.h>

namespace App {

	struct Device
	{
		Microsoft::WRL::ComPtr<ID3D11Device> device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
		Microsoft::WRL::ComPtr<IDXGIFactory> factory;
		D3D_FEATURE_LEVEL featureLevel;
	};

	struct Swapchain
	{
		Microsoft::WRL::ComPtr<IDXGISwapChain4> swapchain;
		DXGI_SWAP_CHAIN_DESC1 swapchainParams;
	};

	Device CreateDevice();
	Swapchain CreateSwapchain(HWND window, Microsoft::WRL::ComPtr<ID3D11Device>& device,
		Microsoft::WRL::ComPtr<IDXGIFactory>& factory, int width, int height);
}
