#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>

namespace App {

	class VertexShader
	{
	public:
		void CreateShader(Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filename);
		void Bind(Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext);

		void CreateLayout(Microsoft::WRL::ComPtr<ID3D11Device>& device);

		Microsoft::WRL::ComPtr<ID3D11VertexShader> GetShaderHandle() const { return m_VertexShader; }
		Microsoft::WRL::ComPtr<ID3D11InputLayout> GetLayout() const { return m_Layout; }
		Microsoft::WRL::ComPtr<ID3DBlob> GetBlob() const { return m_Blob; }

		void AddInputDesc(const char* name, uint32_t index, DXGI_FORMAT format, uint32_t inputSlot, uint32_t offset,
			D3D11_INPUT_CLASSIFICATION classification, uint32_t dataStepRate)
		{

			D3D11_INPUT_ELEMENT_DESC desc;
			desc.SemanticName = name;
			desc.SemanticIndex = index;
			desc.Format = format;
			desc.InputSlot = inputSlot;
			desc.AlignedByteOffset = offset;
			desc.InputSlotClass = classification;
			desc.InstanceDataStepRate = dataStepRate;

			m_InputDescArray.push_back(desc);
		}

		const std::vector<D3D11_INPUT_ELEMENT_DESC>& GetInputDesc() const { return m_InputDescArray; }

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_Layout;
		Microsoft::WRL::ComPtr<ID3DBlob> m_Blob;

		std::vector<D3D11_INPUT_ELEMENT_DESC> m_InputDescArray;
	};

	class PixelShader
	{
	public:
		void CreateShader(Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filename);
		void Bind(Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext);

		Microsoft::WRL::ComPtr<ID3D11PixelShader> GetShaderHandle() const { return m_PixelShader; }
		Microsoft::WRL::ComPtr<ID3DBlob> GetBlob() const { return m_Blob; }

	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
		Microsoft::WRL::ComPtr<ID3DBlob> m_Blob;
	};


	class ComputeShader
	{
	public:
		void CreateShader(Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filename);
		void Bind(Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext);

		Microsoft::WRL::ComPtr<ID3D11ComputeShader> GetShaderHandle() const { return m_ComputeShader; }
		Microsoft::WRL::ComPtr<ID3DBlob> GetBlob() const { return m_Blob; }

	private:
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_ComputeShader;
		Microsoft::WRL::ComPtr<ID3DBlob> m_Blob;
	};
}

