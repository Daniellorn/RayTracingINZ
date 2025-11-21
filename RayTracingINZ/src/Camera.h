#pragma once

#include <DirectXMath.h>
#include <Windows.h>

namespace App {

	class Camera
	{
	public:

		Camera(HWND window, float fov, float nearClip, float farClip, int width, int height);

		const DirectX::XMFLOAT4X4 GetInverseProjection() const { return m_InverseProjection; }
		const DirectX::XMFLOAT4X4 GetInverseView() const { return m_InverseView; }
		const DirectX::XMFLOAT3 GetPosition() const { return m_Position; }
		const DirectX::XMFLOAT3 GetDirection() const { return m_ForwardDirection; }

		bool OnUpdate(float ts);
		void OnResize(int width, int height);

	private:

		void RecalculateView();
		void RecalculateProjection();

	private:
		DirectX::XMFLOAT4X4 m_InverseProjection;
		DirectX::XMFLOAT4X4 m_InverseView;

		DirectX::XMFLOAT3 m_Position;
		DirectX::XMFLOAT3 m_ForwardDirection;

		HWND m_Window;

		float m_FOV = 45.0f;
		float m_NearClip = 0.1f;
		float m_FarClip = 100.0f;
		float m_Sensitivity = 0.002f;

		int m_Width, m_Height;

		POINT m_MouseLastPosition;

	};
}

