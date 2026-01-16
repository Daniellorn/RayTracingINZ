#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "Utils.h"

namespace App {
	
	template<typename T>
	class ConstantBuffer
	{
	public:

		ConstantBuffer() = default;

		ConstantBuffer(ID3D11Device* device, const T& data):
			m_Data(data)
		{
			m_ConstantBufferConfig.ByteWidth = (sizeof(T) + 0xF) & 0xFFFFFFF0;
			m_ConstantBufferConfig.Usage = D3D11_USAGE_DYNAMIC;
			m_ConstantBufferConfig.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			m_ConstantBufferConfig.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			m_ConstantBufferConfig.StructureByteStride = 0;
			m_ConstantBufferConfig.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA constantBufferData;
			constantBufferData.pSysMem = &data;
			constantBufferData.SysMemPitch = 0;
			constantBufferData.SysMemSlicePitch = 0;

			CHECK(device->CreateBuffer(&m_ConstantBufferConfig, &constantBufferData, &m_ConstantBuffer));
		}

		void BindPS(ID3D11DeviceContext* deviceContext, const T& data, uint32_t slot)
		{
			if (memcmp(&m_Data, &data, sizeof(T)) != 0)
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				deviceContext->Map(m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				memcpy(mappedResource.pData, &data, sizeof(data));
				deviceContext->Unmap(m_ConstantBuffer.Get(), 0);

				m_Data = data;
			}

			deviceContext->PSSetConstantBuffers(slot, 1, m_ConstantBuffer.GetAddressOf());
		}

		void BindCS(ID3D11DeviceContext* deviceContext, const T& data, uint32_t slot)
		{
			if (memcmp(&m_Data, &data, sizeof(T)) != 0)
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				deviceContext->Map(m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				memcpy(mappedResource.pData, &data, sizeof(data));
				deviceContext->Unmap(m_ConstantBuffer.Get(), 0);

				m_Data = data;
			}


			deviceContext->CSSetConstantBuffers(slot, 1, m_ConstantBuffer.GetAddressOf());
		}

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
		D3D11_BUFFER_DESC m_ConstantBufferConfig{};

		T m_Data;
	};

	template<typename T>
	class StructuredBuffer
	{
	public:

		StructuredBuffer() = default;

		StructuredBuffer(ID3D11Device* device, const std::vector<T>& data) : m_Data(data)
		{
			m_StructuredBufferConfig.ByteWidth = sizeof(T) * (UINT)data.size();
			m_StructuredBufferConfig.Usage = D3D11_USAGE_DYNAMIC;
			m_StructuredBufferConfig.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			m_StructuredBufferConfig.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			m_StructuredBufferConfig.StructureByteStride = sizeof(T);
			m_StructuredBufferConfig.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;


			D3D11_SUBRESOURCE_DATA structuredBufferData;
			structuredBufferData.pSysMem = data.data();
			structuredBufferData.SysMemPitch = 0;
			structuredBufferData.SysMemSlicePitch = 0;

			CHECK(device->CreateBuffer(&m_StructuredBufferConfig, &structuredBufferData, m_StructuredBuffer.GetAddressOf()));

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = (UINT)data.size();

			CHECK(device->CreateShaderResourceView(m_StructuredBuffer.Get(), &srvDesc, m_SRV.GetAddressOf()));

		}

		void Update(ID3D11DeviceContext* deviceContext, ID3D11Device* device, const std::vector<T>& data)
		{
			if (data.empty())
			{
				return;
			}

			D3D11_BUFFER_DESC desc;
			m_StructuredBuffer->GetDesc(&desc);

			if (desc.ByteWidth < sizeof(T) * data.size())
			{
				desc.ByteWidth = (UINT)(sizeof(T) * data.size());
				m_StructuredBuffer.Reset();

				D3D11_SUBRESOURCE_DATA initData = { data.data(), 0, 0 };
				device->CreateBuffer(&desc, &initData, m_StructuredBuffer.GetAddressOf());
				UpdateSRV(device);
			}
			else
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				CHECK(deviceContext->Map(m_StructuredBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
				memcpy(mappedResource.pData, data.data(), sizeof(T) * data.size());
				deviceContext->Unmap(m_StructuredBuffer.Get(), 0);
			}

			m_Data = data;
		}

		void UpdateSRV(ID3D11Device* device)
		{
			m_SRV.Reset();

			D3D11_BUFFER_DESC bufferDesc;
			m_StructuredBuffer->GetDesc(&bufferDesc);

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = bufferDesc.ByteWidth / sizeof(T);

			CHECK(device->CreateShaderResourceView(m_StructuredBuffer.Get(), &srvDesc, m_SRV.GetAddressOf()));

		}

		void BindPS(ID3D11DeviceContext* deviceContext, uint32_t slot)
		{
			deviceContext->PSSetShaderResources(slot, 1, m_SRV.GetAddressOf());
		}

		void BindCS(ID3D11DeviceContext* deviceContext, uint32_t slot)
		{
			deviceContext->CSSetShaderResources(slot, 1, m_SRV.GetAddressOf());
		}

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_StructuredBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
		D3D11_BUFFER_DESC m_StructuredBufferConfig{};

		std::vector<T> m_Data;
	};

}

