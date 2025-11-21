#pragma once

#include <windows.h>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include "Device.h"
#include "Shader.h"
#include "Scene.h"
#include "Uniforms.h"
#include "Camera.h"

namespace App {

	const uint32_t workGroupSizeX = 16;
	const uint32_t workGroupSizeY = 16;

	struct Texture
	{
		Microsoft::WRL::ComPtr< ID3D11Texture2D> renderTexture;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> UAV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
		D3D11_SAMPLER_DESC samplerDesc;
		D3D11_TEXTURE2D_DESC texDesc;

	};

	struct alignas(16) CameraBuffer
	{
		DirectX::XMFLOAT4X4 invProjectionMartix;
		DirectX::XMFLOAT4X4 invViewMatrix;
		DirectX::XMFLOAT3 cameraPosition;
	};

	class Renderer
	{
	public:

		Renderer(HWND handle, int width, int height, Scene scene);

		~Renderer() = default;

		void InitRenderer(Camera& camera);
		void OnRender();
		void ClearBuffer();
		void EndFrame();

		void Draw(float ts);

		void Resize(int width, int height);

		Device& GetDevice() { return m_Device; }
		Texture& GetPSTexture() { return m_PSTexture; }

		void UpdateSceneBuffers(Scene& scene);

	private:
		HWND m_WindowHandle;
		int m_Width;
		int m_Height;

		Device m_Device;
		Swapchain m_Swapchain;
		Texture m_CSTexture;
		Texture m_PSTexture;
		VertexShader m_VS;
		PixelShader m_PS;
		ComputeShader m_CS;
		D3D11_VIEWPORT m_ViewPort;

		Microsoft::WRL::ComPtr< ID3D11Texture2D> m_BackbufferTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTarget;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_BackBufferRenderTarget;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
		D3D11_BUFFER_DESC m_VertexBufferConfig;

		CameraBuffer m_CameraBuffer;
		Scene m_Scene;
		Camera* m_Camera = nullptr;


		ConstantBuffer<CameraBuffer> m_CameraConstantBuffer;
		StructuredBuffer<Sphere> m_SpheresBuffer;
		StructuredBuffer<Material> m_MaterialsBuffer;

		struct Vertex
		{
			Vertex(float x, float y, float r, float g, float b) :
				pos(x, y), color(r, g, b)
			{

			}

			Vertex(float x, float y, float u, float v) :
				pos(x, y), uv(u, v)
			{

			}

			Vertex(float x, float y) :
				pos(x, y)
			{

			}

			DirectX::XMFLOAT2 pos{};
			DirectX::XMFLOAT2 uv{};
			DirectX::XMFLOAT3 color{};
		};

		uint32_t m_NumGroupsX;
		uint32_t m_NumGroupsY;

	};

};

