#pragma once

#include <memory>

#include "AppWindow.h"
#include "Renderer.h"
#include "Scene.h"


namespace App {

	class Application
	{
	public:

		Application() : m_Renderer(nullptr) {}

		void Init();
		void Run();

	private:

		HWND m_WindowHandle;
		WindowSpecification m_WindowSpec;

		std::unique_ptr<Renderer> m_Renderer;

		Scene m_Scene;
	};
}

