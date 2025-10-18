#include "Application.h"

namespace App {

	void Application::Init()
	{
		m_WindowHandle = CreateWindowApp(m_WindowSpec);
		m_Renderer = std::make_unique<Renderer>(m_WindowHandle, m_WindowSpec.width, m_WindowSpec.height);

	}
	void Application::Run()
	{
		Init();

		ShowWindow(m_WindowHandle, SW_SHOW);

		MSG msg{};

		m_Renderer->InitRenderer();

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			m_Renderer->ClearBuffer();
			m_Renderer->Draw();
			m_Renderer->EndFrame();
		}
	}
}