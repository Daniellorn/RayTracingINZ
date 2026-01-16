#pragma once

#include <memory>

#include "AppWindow.h"
#include "Renderer.h"
#include "Scene.h"
#include "Timer.h"

#include <vector>
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

		std::vector<std::string> m_FileNames;

		Timer m_Timer;

		inline static std::string s_FolderPath = "res/models/";

		bool m_Running;
		float m_LastRenderTime = 0.0f;
	};
}

