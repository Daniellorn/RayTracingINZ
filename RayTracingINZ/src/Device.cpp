#include "Device.h"

#include "Utils.h"

using Microsoft::WRL::ComPtr;

namespace App {
	
	Device App::CreateDevice()
	{
		Device result{};

		CHECK(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(result.factory.GetAddressOf())));

		D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

		CHECK(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 
			D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG,
			featureLevelArray, 2, D3D11_SDK_VERSION,
			&result.device, &result.featureLevel, &result.deviceContext));

		return result;
	}
	Swapchain CreateSwapchain(HWND window, ComPtr<ID3D11Device>& device, 
		ComPtr<IDXGIFactory>& factory, int width, int height)
	{
		Swapchain result{};

		result.swapchainParams.Width = width;
		result.swapchainParams.Height = height;
		result.swapchainParams.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		result.swapchainParams.Stereo = false;
		result.swapchainParams.SampleDesc.Quality = 0;
		result.swapchainParams.SampleDesc.Count = 1;
		result.swapchainParams.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		result.swapchainParams.BufferCount = 2;
		result.swapchainParams.Scaling = DXGI_SCALING_NONE;
		result.swapchainParams.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		result.swapchainParams.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		result.swapchainParams.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;;

		ComPtr< IDXGIFactory6> factory6;
		ComPtr<IDXGISwapChain1> tempSwapChain;

		CHECK(factory.As(&factory6));

		CHECK(factory6->CreateSwapChainForHwnd(device.Get(), window, &result.swapchainParams,
			nullptr, nullptr, tempSwapChain.GetAddressOf()));

		CHECK(tempSwapChain.As(&result.swapchain));

		return result;

	}
}
