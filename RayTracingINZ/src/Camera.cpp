#include "Camera.h"

using namespace DirectX;

#define KEY_PRESSED(vk_code) (GetAsyncKeyState(vk_code) & 0x8000)

namespace App {
	
	Camera::Camera(HWND window, float fov, float nearClip, float farClip, int width, int height):
		m_Window(window), m_FOV(fov), m_NearClip(nearClip), m_FarClip(farClip), m_Width(width), m_Height(height)
	{
		m_ForwardDirection = XMFLOAT3{ 0.0f, 0.0f, 1.0f };
		m_Position = XMFLOAT3{ 0.0f, 0.0f, -3.0f };

		XMStoreFloat4x4(&m_InverseProjection, DirectX::XMMatrixIdentity());
		XMStoreFloat4x4(&m_InverseView, DirectX::XMMatrixIdentity());
		XMStoreFloat4x4(&m_InverseProjection, DirectX::XMMatrixIdentity());
		XMStoreFloat4x4(&m_InverseView, DirectX::XMMatrixIdentity());

		RecalculateProjection();
		RecalculateView();
	}

	bool Camera::OnUpdate(float ts)
	{
		if (!KEY_PRESSED(VK_RBUTTON))
		{
			ShowCursor(true);
			ClipCursor(nullptr);
			return false;
		}


		POINT center;
		center.x = m_Width / 2;
		center.y = m_Height / 2;
		
		POINT mousePosition;
		GetCursorPos(&mousePosition);

		ClientToScreen(m_Window, &center);

		float deltaX = (mousePosition.x - center.x) * m_Sensitivity;
		float deltaY = (mousePosition.y - center.y) * m_Sensitivity;

		SetCursorPos(center.x, center.y);

		const XMVECTORF32 upDirection = { 0.0f, 1.0f, 0.0f, 0.0f };

		XMVECTOR forwardDirection = XMLoadFloat3(&m_ForwardDirection);
		XMVECTOR position = XMLoadFloat3(&m_Position);

		XMVECTOR rightDirection = XMVector3Normalize(XMVector3Cross(forwardDirection, upDirection));

		float speed = 5.0f;
		bool moved = false;

		if (KEY_PRESSED('W'))
		{
			position += forwardDirection * speed * ts;
			moved = true;
		}
		if (KEY_PRESSED('S'))
		{
			position -= forwardDirection * speed * ts;
			moved = true;
		}
		if (KEY_PRESSED('D'))
		{
			position -= rightDirection * speed * ts;
			moved = true;
		}
		if (KEY_PRESSED('A'))
		{
			position += rightDirection * speed * ts;
			moved = true;
		}
		if (KEY_PRESSED(VK_SPACE))
		{
			position += upDirection * speed * ts;
			moved = true;
		}
		if (KEY_PRESSED(VK_SHIFT))
		{
			position -= upDirection * speed * ts;
			moved = true;
		}


		if (deltaX != 0 || deltaY != 0)
		{
			float pitchDelta = deltaY * 0.4f;
			float yawDelta = deltaX * 0.4f;

			XMVECTOR pitchQuat = XMQuaternionRotationAxis(rightDirection, -pitchDelta);
			XMVECTOR yawQuat = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), yawDelta);

			XMVECTOR q = XMQuaternionMultiply(yawQuat, pitchQuat); // wynikiem jest q2 * q1 dlatego yaw jest pierwsze
			q = XMQuaternionNormalize(q);

			forwardDirection = XMVector3Rotate(forwardDirection, q);

			moved = true;
		}

		if (moved)
		{
			RecalculateView();
		}

		XMStoreFloat3(&m_ForwardDirection, forwardDirection);
		XMStoreFloat3(&m_Position, position);


		return moved;

	}

	void Camera::OnResize(int width, int height)
	{
		if (m_Width == width && m_Height == height)
		{
			return;
		}

		m_Width = width;
		m_Height = height;

		RecalculateView();
		RecalculateProjection();
	}

	void Camera::RecalculateView()
	{
		XMVECTOR position = XMLoadFloat3(&m_Position);
		XMVECTOR forwardDirection = XMLoadFloat3(&m_ForwardDirection);
		XMVECTOR target = position + forwardDirection;

		XMMATRIX view = XMMatrixLookAtLH(position, target, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		XMMATRIX inverseView = XMMatrixInverse(nullptr, view);

		XMStoreFloat4x4(&m_InverseView, inverseView);
	}

	void Camera::RecalculateProjection()
	{
		float aspectRatio = (float)m_Width / (float)m_Height;
		XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FOV), aspectRatio, m_NearClip, m_FarClip);
		XMMATRIX inverseProjection = XMMatrixInverse(nullptr, projection);

		XMStoreFloat4x4(&m_InverseProjection, inverseProjection);
	}

}