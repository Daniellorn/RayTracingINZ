#include "Application.h"
#include <DirectXMath.h>

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include <Windows.h>

#define KEY_PRESSED(vk_code) (GetAsyncKeyState(vk_code) & 0x8000)

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


        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static bool dockSpaceOpen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        //TODO: Jakis warning jest
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}


			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGui::ShowDemoWindow();

            m_Renderer->ClearBuffer();
            m_Renderer->Draw();

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen)
            {
                const ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->WorkPos);
                ImGui::SetNextWindowSize(viewport->WorkSize);
                ImGui::SetNextWindowViewport(viewport->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }
            else
            {
                dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
            }

            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;
            if (!opt_padding)
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));


            ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
            if (!opt_padding)
                ImGui::PopStyleVar();

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("Options"))
                {

                    if (ImGui::MenuItem("Exit"))
                    {
                        Close();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
			//ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_FirstUseEver);
            ImGui::Begin("Viewport");
			
            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0 &&
                (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y))
			{

			    m_Renderer->Resize((int)viewportPanelSize.x, (int)viewportPanelSize.y);
			    m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
			}
			auto textureID = m_Renderer->GetPSTexture();
			ImGui::Image((ImTextureID)textureID.SRV.Get(), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });
			
            ImGui::End();

            ImGui::Begin("Settings");
            ImGui::End();

            ImGui::End();
            
			ImGui::Render();

			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			m_Renderer->EndFrame();

		}

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
	}

    void Application::Close()
    {
        PostQuitMessage(0);
    }
}