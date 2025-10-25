#include "Shader.h"
#include "Utils.h"

#include <D3DCompiler.h>

using Microsoft::WRL::ComPtr;

namespace App {

	void VertexShader::CreateShader(ComPtr<ID3D11Device>& device, const std::wstring& filename)
	{
		CHECK(D3DReadFileToBlob(filename.c_str(), &m_Blob));

		CHECK(device->CreateVertexShader(m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(), nullptr, &m_VertexShader));
	}

	void VertexShader::Bind(Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext)
	{
		deviceContext->VSSetShader(m_VertexShader.Get(), 0, 0);
		deviceContext->IASetInputLayout(m_Layout.Get());
	}

	void VertexShader::CreateLayout(Microsoft::WRL::ComPtr<ID3D11Device>& device)
	{
		CHECK(device->CreateInputLayout(m_InputDescArray.data(), (UINT)m_InputDescArray.size(),
			m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(), &m_Layout));
	}

	void VertexShader::AddInputDesc(std::initializer_list<VertexBufferLayout> list)
	{
		for (const auto& input : list)
		{
			D3D11_INPUT_ELEMENT_DESC desc;
			desc.SemanticName = input.name;
			desc.SemanticIndex = input.index;
			desc.Format = input.format;
			desc.InputSlot = input.inputSlot;
			desc.AlignedByteOffset = input.offset;
			desc.InputSlotClass = input.classification;
			desc.InstanceDataStepRate = input.dataStepRate;

			m_InputDescArray.push_back(desc);
		}
	}

	void PixelShader::CreateShader(ComPtr<ID3D11Device>& device, const std::wstring& filename)
	{
		CHECK(D3DReadFileToBlob(filename.c_str(), &m_Blob));

		CHECK(device->CreatePixelShader(m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(), nullptr, &m_PixelShader));
	}

	void PixelShader::Bind(ComPtr<ID3D11DeviceContext> deviceContext)
	{
		deviceContext->PSSetShader(m_PixelShader.Get(), 0, 0);
	}

	void ComputeShader::CreateShader(ComPtr<ID3D11Device>& device, const std::wstring& filename)
	{
		CHECK(D3DReadFileToBlob(filename.c_str(), &m_Blob));

		CHECK(device->CreateComputeShader(m_Blob->GetBufferPointer(), m_Blob->GetBufferSize(), nullptr, &m_ComputeShader));
	}

	void ComputeShader::Bind(ComPtr<ID3D11DeviceContext> deviceContext)
	{
		deviceContext->CSSetShader(m_ComputeShader.Get(), 0, 0);
	}

}