#include "Application.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace App {

	void Application::Init() 
	{
		m_Scene.AddObject(Sphere(0.0f, 1.0f, 0.0f, 1.0f, 0, static_cast<int>(Model::DIFFUSE)));
		m_Scene.AddObject(Sphere(0.0f, -1.0f, 0.0f, 1.0f, 1, static_cast<int>(Model::DIFFUSE)));
		m_Scene.AddMaterial(Material(XMFLOAT4{ 0.0f, 0.0f, 1.0f, 0.0f }, XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f), 1.0f, 0.0f, 1.0f));
		m_Scene.AddMaterial(Material(XMFLOAT4{ 1.0f, 0.0f, 0.0f, 0.0f }, XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f), 1.0f, 0.0f, 1.0f));


		m_WindowHandle = CreateWindowApp(m_WindowSpec);
		m_Renderer = std::make_unique<Renderer>(m_WindowHandle, m_WindowSpec.width, m_WindowSpec.height, m_Scene);

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