#pragma once

#include <memory>

#include "AppWindow.h"
#include "Renderer.h"
#include "Scene.h"

#include <DirectXMath.h>


namespace App {

	class Application
	{
	public:

		Application() : m_Renderer(nullptr) {}

		void Init();
		void Run();
		void Close();

	private:

		HWND m_WindowHandle;
		WindowSpecification m_WindowSpec;

		std::unique_ptr<Renderer> m_Renderer;
		DirectX::XMFLOAT2 m_ViewportSize;

		Scene m_Scene;

		bool m_Running;
	};
}

