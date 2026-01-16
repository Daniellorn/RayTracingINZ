#include "Renderer.h"
#include "Utils.h"

#include <DirectXColors.h>


namespace App {

	Renderer::Renderer(HWND handle, int width, int height) : m_WindowHandle(handle), m_Width(width),
		m_Height(height)
	{
		m_NumGroupsX = (m_Width + workGroupSizeX - 1) / workGroupSizeX;
		m_NumGroupsY = (m_Height + workGroupSizeY - 1) / workGroupSizeY;

		//InitRenderer();
	}

	void Renderer::InitRenderer(Camera& camera, Scene& scene)
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
		m_CSTexture.texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		m_CSTexture.texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

		// tekstura, uav, srv do compute shadera
		CHECK(m_Device.device->CreateTexture2D(&m_CSTexture.texDesc, nullptr, m_CSTexture.renderTexture.GetAddressOf()));
		CHECK(m_Device.device->CreateUnorderedAccessView(m_CSTexture.renderTexture.Get(), nullptr, &m_CSTexture.UAV));
		CHECK(m_Device.device->CreateShaderResourceView(m_CSTexture.renderTexture.Get(), nullptr, &m_CSTexture.SRV));

		// tekstura akumulacyjna do compute shadera
		m_AccumulationTexture.texDesc = m_CSTexture.texDesc;
		CHECK(m_Device.device->CreateTexture2D(&m_AccumulationTexture.texDesc, nullptr, m_AccumulationTexture.renderTexture.GetAddressOf()));
		CHECK(m_Device.device->CreateUnorderedAccessView(m_AccumulationTexture.renderTexture.Get(), nullptr, &m_AccumulationTexture.UAV));

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
		m_PSTexture.texDesc.Width = m_Width;
		m_PSTexture.texDesc.Height = m_Height;
		m_PSTexture.texDesc.MipLevels = 1;
		m_PSTexture.texDesc.ArraySize = 1;
		m_PSTexture.texDesc.Usage = D3D11_USAGE_DEFAULT;
		m_PSTexture.texDesc.CPUAccessFlags = 0;
		m_PSTexture.texDesc.MiscFlags = 0;
		m_PSTexture.texDesc.SampleDesc.Count = 1;
		m_PSTexture.texDesc.SampleDesc.Quality = 0;
		m_PSTexture.texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		m_PSTexture.texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		CHECK(m_Device.device->CreateTexture2D(&m_PSTexture.texDesc, nullptr, m_PSTexture.renderTexture.GetAddressOf()));
		CHECK(m_Device.device->CreateShaderResourceView(m_PSTexture.renderTexture.Get(), nullptr, &m_PSTexture.SRV));
		CHECK(m_Device.device->CreateRenderTargetView(m_PSTexture.renderTexture.Get(), nullptr, &m_RenderTarget));

		m_Swapchain.swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(m_BackbufferTexture.GetAddressOf()));
		CHECK(m_Device.device->CreateRenderTargetView(m_BackbufferTexture.Get(), nullptr, &m_BackBufferRenderTarget));

		m_Camera = &camera;
		m_Scene = &scene;

		auto& spheres = scene.GetSpheres();
		auto& materials = scene.GetMaterials();
		auto& triangles = scene.GetTriangles();
		auto& models = scene.GetModels();
		auto& renderConfiguration = scene.GetRenderConfiguration();
		auto& bvhnodes = scene.GetBVHNodes();
		auto& triIndexes = scene.GetTriIndexes();

		m_CameraBuffer.cameraPosition = m_Camera->GetPosition();
		m_CameraBuffer.invProjectionMartix = m_Camera->GetInverseProjection();
		m_CameraBuffer.invViewMatrix = m_Camera->GetInverseView();

		m_CameraConstantBuffer = ConstantBuffer <CameraBuffer>(m_Device.device.Get(), m_CameraBuffer);
		m_SpheresBuffer = StructuredBuffer<Sphere>(m_Device.device.Get(), spheres);
		m_MaterialsBuffer = StructuredBuffer<Material>(m_Device.device.Get(), materials);
		m_TrianglesBuffer = StructuredBuffer<Triangle>(m_Device.device.Get(), triangles);
		m_ModelsBuffer = StructuredBuffer<Model>(m_Device.device.Get(), models);
		m_SceneConfigurationBuffer = ConstantBuffer<SceneConfiguration>(m_Device.device.Get(), m_Scene->GetSceneConfiguration());
		m_RenderDataBuffer = ConstantBuffer<RenderConfiguration>(m_Device.device.Get(), renderConfiguration);
		m_BVHNodeBuffer = StructuredBuffer<BVHNode>(m_Device.device.Get(), bvhnodes);
		m_TriIndexesBuffer = StructuredBuffer<int>(m_Device.device.Get(), triIndexes);


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
		m_VS.AddInputDesc({
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } });
		m_VS.CreateLayout(m_Device.device);

		//PixelShader
		m_PS.CreateShader(m_Device.device, L"res/shaders/PixelShader.fxc");

		//ComputeShader
		m_CS.CreateShader(m_Device.device, L"res/shaders/ComputeShader.fxc");

		m_ViewPort.Width = (float)m_Width;
		m_ViewPort.Height = (float)m_Height;
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
	void Renderer::Draw(float ts)
	{
		auto& renderConfiguration = m_Scene->GetRenderConfiguration();
		bool cameraMoved = m_Camera->OnUpdate(ts);

		if ((cameraMoved || renderConfiguration.frameIndex == 1) && renderConfiguration.accumulate == 1)
		{
			ClearTex(m_Device.deviceContext, m_AccumulationTexture.UAV);
			renderConfiguration.frameIndex = 1;
		}

		m_CameraBuffer.cameraPosition = m_Camera->GetPosition();
		m_CameraBuffer.invProjectionMartix = m_Camera->GetInverseProjection();
		m_CameraBuffer.invViewMatrix = m_Camera->GetInverseView();

		ID3D11ShaderResourceView* nullSRV = nullptr;
		m_Device.deviceContext->PSSetShaderResources(0, 1, &nullSRV);

		m_SpheresBuffer.BindCS(m_Device.deviceContext.Get(), 0);
		m_MaterialsBuffer.BindCS(m_Device.deviceContext.Get(), 1);
		m_TrianglesBuffer.BindCS(m_Device.deviceContext.Get(), 2);
		m_ModelsBuffer.BindCS(m_Device.deviceContext.Get(), 3);
		m_TriIndexesBuffer.BindCS(m_Device.deviceContext.Get(), 4);
		m_BVHNodeBuffer.BindCS(m_Device.deviceContext.Get(), 5);
		m_CameraConstantBuffer.BindCS(m_Device.deviceContext.Get(), m_CameraBuffer, 0);
		m_SceneConfigurationBuffer.BindCS(m_Device.deviceContext.Get(), m_Scene->GetSceneConfiguration(), 1);
		m_RenderDataBuffer.BindCS(m_Device.deviceContext.Get(), renderConfiguration, 2);


		const UINT stride = sizeof(Vertex);
		const UINT offset = 0;
		m_Device.deviceContext->CSSetUnorderedAccessViews(0, 1, m_CSTexture.UAV.GetAddressOf(), nullptr);
		m_Device.deviceContext->CSSetUnorderedAccessViews(1, 1, m_AccumulationTexture.UAV.GetAddressOf(), nullptr);
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

		ID3D11RenderTargetView* nullRTV = nullptr;
		m_Device.deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
		ID3D11ShaderResourceView* const pSRV[1] = { NULL };
		m_Device.deviceContext->PSSetShaderResources(0, 1, pSRV);

		m_Device.deviceContext->OMSetRenderTargets(1, m_BackBufferRenderTarget.GetAddressOf(), nullptr);
		//m_Device.deviceContext->ClearRenderTargetView(m_BackBufferRenderTarget.Get(), DirectX::Colors::DarkGray);

	}
	void Renderer::Resize(int width, int height)
	{
		m_Device.deviceContext->ClearState();
		m_Device.deviceContext->Flush();

		m_RenderTarget.Reset();
		m_BackBufferRenderTarget.Reset();
		m_BackbufferTexture.Reset();
		m_Swapchain.swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

		CHECK(m_Swapchain.swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D),
			reinterpret_cast<void**>(m_BackbufferTexture.GetAddressOf())));
		CHECK(m_Device.device->CreateRenderTargetView(m_BackbufferTexture.Get(), nullptr, &m_BackBufferRenderTarget));

		m_CSTexture.UAV.Reset();
		m_CSTexture.SRV.Reset();
		m_CSTexture.renderTexture.Reset();

		m_AccumulationTexture.UAV.Reset();
		//m_AccumulationTexture.SRV.Reset();
		m_AccumulationTexture.renderTexture.Reset();

		m_PSTexture.renderTexture.Reset();
		m_PSTexture.SRV.Reset();

		// tekstura do compute shadera
		m_CSTexture.texDesc.Width = width;
		m_CSTexture.texDesc.Height = height;

		m_AccumulationTexture.texDesc.Width = width;
		m_AccumulationTexture.texDesc.Height = height;

		m_PSTexture.texDesc.Width = width;
		m_PSTexture.texDesc.Height = height;

		// tekstura, uav, srv do compute shadera
		CHECK(m_Device.device->CreateTexture2D(&m_CSTexture.texDesc, nullptr, m_CSTexture.renderTexture.GetAddressOf()));
		CHECK(m_Device.device->CreateUnorderedAccessView(m_CSTexture.renderTexture.Get(), nullptr, &m_CSTexture.UAV));
		CHECK(m_Device.device->CreateShaderResourceView(m_CSTexture.renderTexture.Get(), nullptr, &m_CSTexture.SRV));

		// tekstura akumulacji
		m_AccumulationTexture.texDesc = m_CSTexture.texDesc;
		CHECK(m_Device.device->CreateTexture2D(&m_AccumulationTexture.texDesc, nullptr, m_AccumulationTexture.renderTexture.GetAddressOf()));
		CHECK(m_Device.device->CreateUnorderedAccessView(m_AccumulationTexture.renderTexture.Get(), nullptr, &m_AccumulationTexture.UAV));

		//PS tekstura
		CHECK(m_Device.device->CreateTexture2D(&m_PSTexture.texDesc, nullptr, m_PSTexture.renderTexture.GetAddressOf()));
		CHECK(m_Device.device->CreateShaderResourceView(m_PSTexture.renderTexture.Get(), nullptr, &m_PSTexture.SRV));
		CHECK(m_Device.device->CreateRenderTargetView(m_PSTexture.renderTexture.Get(), nullptr, &m_RenderTarget));

		m_ViewPort.Width = (float)width;
		m_ViewPort.Height = (float)height;
		m_ViewPort.MinDepth = 0;
		m_ViewPort.MaxDepth = 1;
		m_ViewPort.TopLeftX = 0;
		m_ViewPort.TopLeftY = 0;

		m_Device.deviceContext->RSSetViewports(1, &m_ViewPort);
		m_Device.deviceContext->OMSetRenderTargets(1, m_RenderTarget.GetAddressOf(), nullptr);

		m_Width = width;
		m_Height = height;

		m_NumGroupsX = (width + workGroupSizeX - 1) / workGroupSizeX;
		m_NumGroupsY = (height + workGroupSizeY - 1) / workGroupSizeY;
	}
	void Renderer::UpdateSceneBuffers(Scene& scene)
	{
		m_SpheresBuffer.Update(m_Device.deviceContext.Get(), m_Device.device.Get(), scene.GetSpheres());
		m_MaterialsBuffer.Update(m_Device.deviceContext.Get(), m_Device.device.Get(), scene.GetMaterials());
		m_TrianglesBuffer.Update(m_Device.deviceContext.Get(), m_Device.device.Get(), scene.GetTriangles());
		m_ModelsBuffer.Update(m_Device.deviceContext.Get(), m_Device.device.Get(), scene.GetModels());
		m_BVHNodeBuffer.Update(m_Device.deviceContext.Get(), m_Device.device.Get(), scene.GetBVHNodes());
		m_TriIndexesBuffer.Update(m_Device.deviceContext.Get(), m_Device.device.Get(), scene.GetTriIndexes());
	}
}