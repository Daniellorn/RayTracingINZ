#include "Renderer.h"
#include "Utils.h"

#include <DirectXColors.h>


namespace App {
	
	Renderer::Renderer(HWND handle, int width, int height) : m_WindowHandle(handle), m_Width(width),
		m_Height(height)
	{
		m_NumGroupsX = (1280 + workGroupSizeX - 1) / workGroupSizeX;
		m_NumGroupsY = (720 + workGroupSizeY - 1) / workGroupSizeY;
	}

	void Renderer::InitRenderer()
	{
		m_Device = CreateDevice();
		m_Swapchain = CreateSwapchain(m_WindowHandle, m_Device.device, m_Device.factory, m_Width, m_Height);


		// tekstura do compute shadera
		m_CSTexture.texDesc.Width = m_Width;
		m_CSTexture.texDesc.Height = m_Height;
		m_CSTexture.texDesc.MipLevels = 1;
		m_CSTexture.texDesc.ArraySize = 1;
		m_CSTexture.texDesc.Usage = D3D11_USAGE_DEFAULT;
		m_CSTexture.texDesc.CPUAccessFlags = 0;
		m_CSTexture.texDesc.MiscFlags = 0;
		m_CSTexture.texDesc.SampleDesc.Count = 1;
		m_CSTexture.texDesc.SampleDesc.Quality = 0;
		m_CSTexture.texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		m_CSTexture.texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

		// tekstura, uav, srv do compute shadera
		CHECK(m_Device.device->CreateTexture2D(&m_CSTexture.texDesc, nullptr, m_CSTexture.renderTexture.GetAddressOf()));
		CHECK(m_Device.device->CreateUnorderedAccessView(m_CSTexture.renderTexture.Get(), nullptr, &m_CSTexture.UAV));
		CHECK(m_Device.device->CreateShaderResourceView(m_CSTexture.renderTexture.Get(), nullptr, &m_CSTexture.SRV));

		//sampler
		m_CSTexture.samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		m_CSTexture.samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		m_CSTexture.samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		m_CSTexture.samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		m_CSTexture.samplerDesc.MipLODBias = 0.0f;
		m_CSTexture.samplerDesc.MaxAnisotropy = 1;
		m_CSTexture.samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		m_CSTexture.samplerDesc.MinLOD = 0;
		m_CSTexture.samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		
		m_Device.device->CreateSamplerState(&m_CSTexture.samplerDesc, m_CSTexture.sampler.GetAddressOf());

		//Tekstura do PS (blit pass)
		m_Swapchain.swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(m_BackbufferTexture.GetAddressOf()));
		CHECK(m_Device.device->CreateRenderTargetView(m_BackbufferTexture.Get(), nullptr, &m_RenderTarget));

		OnRender();

	}
	void Renderer::OnRender()
	{
		Vertex vertices[] = {
			{ -1.0f, -1.0f },
			{ -1.0f,  3.0f },
			{  3.0f, -1.0f }
		};

		//VertexBuffer
		m_VertexBufferConfig.ByteWidth = sizeof(vertices);
		m_VertexBufferConfig.StructureByteStride = sizeof(Vertex);
		m_VertexBufferConfig.Usage = D3D11_USAGE_DEFAULT;
		m_VertexBufferConfig.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		m_VertexBufferConfig.CPUAccessFlags = 0;
		m_VertexBufferConfig.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		vertexBufferData.pSysMem = vertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		CHECK(m_Device.device->CreateBuffer(&m_VertexBufferConfig, &vertexBufferData, &m_VertexBuffer));

		//VertexShader
		m_VS.CreateShader(m_Device.device, L"res/shaders/VertexShader.fxc");
		m_VS.AddInputDesc("POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
		m_VS.CreateLayout(m_Device.device);

		//PixelShader
		m_PS.CreateShader(m_Device.device, L"res/shaders/PixelShader.fxc");

		//ComputeShader
		m_CS.CreateShader(m_Device.device, L"res/shaders/ComputeShader.fxc");

		m_ViewPort.Width = m_Width;
		m_ViewPort.Height = m_Height;
		m_ViewPort.MinDepth = 0;
		m_ViewPort.MaxDepth = 1;
		m_ViewPort.TopLeftX = 0;
		m_ViewPort.TopLeftY = 0;
	}
	void Renderer::ClearBuffer()
	{
		m_Device.deviceContext->ClearRenderTargetView(m_RenderTarget.Get(), DirectX::Colors::DarkGray);
	}
	void Renderer::EndFrame()
	{
		m_Swapchain.swapchain->Present(1u, 0u);
	}
	void Renderer::Draw()
	{
		ID3D11ShaderResourceView* nullSRV = nullptr;
		m_Device.deviceContext->PSSetShaderResources(0, 1, &nullSRV);

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0;
		m_Device.deviceContext->CSSetUnorderedAccessViews(0, 1, m_CSTexture.UAV.GetAddressOf(), nullptr);
		m_CS.Bind(m_Device.deviceContext.Get());
		m_Device.deviceContext->Dispatch(m_NumGroupsX, m_NumGroupsY, 1);

		ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
		m_Device.deviceContext->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

		m_Device.deviceContext->OMSetRenderTargets(1, m_RenderTarget.GetAddressOf(), nullptr);
		m_Device.deviceContext->RSSetViewports(1, &m_ViewPort);

		m_Device.deviceContext->PSSetShaderResources(0, 1, m_CSTexture.SRV.GetAddressOf());
		m_Device.deviceContext->PSSetSamplers(0, 1, m_CSTexture.sampler.GetAddressOf());

		m_Device.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_Device.deviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);


		m_VS.Bind(m_Device.deviceContext.Get());
		m_PS.Bind(m_Device.deviceContext.Get());

		m_Device.deviceContext->Draw(3, 0);
	}
}