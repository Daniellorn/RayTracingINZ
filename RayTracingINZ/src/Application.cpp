#include "Application.h"
#include <DirectXMath.h>

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

using namespace DirectX;

namespace App {

	void Application::Init() 
	{
		m_Scene.AddObject(Sphere(0.0f, 1.0f, 0.0f, 1.0f, 0, static_cast<int>(Model::DIFFUSE)));
		m_Scene.AddObject(Sphere(0.0f, -1.0f, 0.0f, 1.0f, 1, static_cast<int>(Model::DIFFUSE)));
		m_Scene.AddMaterial(Material(XMFLOAT4{ 0.0f, 0.0f, 1.0f, 0.0f }, XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f), 1.0f, 0.0f, 1.0f));
		m_Scene.AddMaterial(Material(XMFLOAT4{ 1.0f, 0.0f, 0.0f, 0.0f }, XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f), 1.0f, 0.0f, 1.0f));


		m_WindowHandle = CreateWindowApp(m_WindowSpec);
		m_Renderer = std::make_unique<Renderer>(m_WindowHandle, m_WindowSpec.clientWidth, m_WindowSpec.clientHeight, m_Scene);

		SetWindowLongPtr(m_WindowHandle, GWLP_USERDATA, (LONG_PTR)m_Renderer.get());

	}
	void Application::Run()
	{
		Init();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		auto device = m_Renderer->GetDevice();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(m_WindowHandle);
		ImGui_ImplDX11_Init(device.device.Get(), device.deviceContext.Get());

		ShowWindow(m_WindowHandle, SW_SHOW);

		MSG msg{};

		//m_Renderer->InitRenderer();

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// (Your code process and dispatch Win32 messages)
// Start the Dear ImGui frame
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGui::ShowDemoWindow(); // Show demo window! :)

			m_Renderer->ClearBuffer();
			m_Renderer->Draw();

			// Rendering
			// (Your code clears your framebuffer, renders your other stuff etc.)
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			// (Your code calls swapchain's Present() function)

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			m_Renderer->EndFrame();

		}
	}
}