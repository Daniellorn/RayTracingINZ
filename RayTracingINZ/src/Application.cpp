#include "Application.h"
#include <DirectXMath.h>
#include <Windows.h>
#include <algorithm>
#include <thread>

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include "Camera.h"

using namespace DirectX;

namespace App {

    float GetDeltaTime()
    {
        static LARGE_INTEGER frequency;
        static LARGE_INTEGER lastTime;
        static bool initialized = false;

        if (!initialized)
        {
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&lastTime);
            initialized = true;
            return 0.0f;
        }

        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);

        float delta = static_cast<float>(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
        lastTime = currentTime;

        delta = std::min<float>(delta, 0.0333f);

        return delta;
    }

	void Application::Init() 
	{

		m_WindowHandle = CreateWindowApp(m_WindowSpec);
		m_Renderer = std::make_unique<Renderer>(m_WindowHandle, m_WindowSpec.clientWidth, m_WindowSpec.clientHeight);

		SetWindowLongPtr(m_WindowHandle, GWLP_USERDATA, (LONG_PTR)m_Renderer.get());

	}
	void Application::Run()
	{
		Init();

        Scene scene({ 4 });
		scene.AddObject(Sphere({-1.7f, -0.7f, -5.1f, 1.0f}, 1.0f, 0, static_cast<int>(Type::DIFFUSE)));
        scene.AddObject(Sphere({ 0.0f, -0.6f, 0.0f, 1.0f }, 1.0f, 1, static_cast<int>(Type::DIFFUSE)));
        scene.AddObject(Sphere({ -17.0f, 5.3f, 0.0f, 1.0f }, 4.0f, 2, static_cast<int>(Type::DIFFUSE)));
		scene.AddObject(Sphere({0.0f, -43.5f, 0.0f, 1.0f}, 42.1f, 0, static_cast<int>(Type::DIFFUSE)));
		scene.AddMaterial(Material(XMFLOAT4{ 0.0f, 0.0f, 1.0f, 0.0f }, XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f), 1.0f, 0.0f, 0.0f));
		scene.AddMaterial(Material(XMFLOAT4{ 1.0f, 0.0f, 0.0f, 0.0f }, XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f), 1.0f, 0.0f, 0.0f));
		scene.AddMaterial(Material(XMFLOAT4{ 1.0f, 0.0f, 1.0f, 0.0f }, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.0f, 5.0f));
		scene.AddMaterial(Material(XMFLOAT4{ 0.0f, 1.0f, 0.0f, 0.0f }, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.0f, 0.0f));

        //scene.AddObject("res/models/model.obj", 3);
        if (!scene.AddObject("res/models/model.obj", 3))
        {

            return;
        }

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


        Camera camera(m_WindowHandle, 45.0f, 0.1f, 100.0f, m_WindowSpec.clientWidth, m_WindowSpec.clientHeight);
        m_Renderer->InitRenderer(camera, scene);
		auto device = m_Renderer->GetDevice();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(m_WindowHandle);
		ImGui_ImplDX11_Init(device.device.Get(), device.deviceContext.Get());

		ShowWindow(m_WindowHandle, SW_SHOW);

		MSG msg{};


        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static bool dockSpaceOpen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        auto& spheres = scene.GetSpheres();
        auto& materials = scene.GetMaterials();
        auto& renderConfiguration = scene.GetRenderConfiguration();

        bool reset = false;
        bool accumulate = false;
        bool stopRendering = false;

        Timer timer;

		while (msg.message != WM_QUIT)
		{
            timer.Reset();
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
            else
            {
                if (IsIconic(m_WindowHandle))
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }

                float ts = GetDeltaTime();

			    ImGui_ImplDX11_NewFrame();
			    ImGui_ImplWin32_NewFrame();
			    ImGui::NewFrame();
			    //ImGui::ShowDemoWindow();


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
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
                ImGui::Begin("Viewport");
			
                m_Renderer->UpdateSceneBuffers(scene);

                m_Renderer->ClearBuffer();
                m_Renderer->Draw(ts);


                ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			    if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0 &&
                    (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y))
			    {

			        m_Renderer->Resize((int)viewportPanelSize.x, (int)viewportPanelSize.y);
                    camera.OnResize((int)viewportPanelSize.x, (int)viewportPanelSize.y);
			        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
			    }
			    auto textureID = m_Renderer->GetPSTexture();
			    ImGui::Image((ImTextureID)textureID.SRV.Get(), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });
			
                ImGui::PopStyleVar();
                ImGui::End();

                ImGui::Begin("Spheres");

                for (size_t i = 0; i < spheres.size(); i++)
                {
                    ImGui::PushID((int)i);
                
                    Sphere& sphere = spheres[i];
                    ImGui::DragFloat3("Position", &sphere.position.x, 0.1f);
                    ImGui::DragFloat("Radius", &sphere.radius, 0.1f, 0.0f, FLT_MAX);
                
                    ImGui::Separator();
                
                    ImGui::PopID();
                }
                ImGui::End();

                ImGui::Begin("Materials");
                for (size_t i = 0; i < materials.size(); i++)
                {
                    ImGui::PushID((int)i);

                    Material& material = materials[i];
                    ImGui::ColorEdit3("Albedo",&material.albedo.x);
                    ImGui::DragFloat("Roughness", &material.roughness, 0.05f, 0.0f, 1.0f);
                    ImGui::DragFloat("Metallic", &material.glossiness, 0.05f, 0.0f, 1.0f);
                    ImGui::ColorEdit3("Emission Color", &material.EmissionColor.x);
                    ImGui::DragFloat("Emission Power", &material.EmissionPower, 0.05f, 0.0f, FLT_MAX);

                    ImGui::Separator();

                    ImGui::PopID();
                }

                ImGui::End();



                ImGui::Begin("Settings");
                ImGui::Text("Last render: %.3fms", m_LastRenderTime);
                ImGui::DragInt("Max bounce", &renderConfiguration.numOfBounces, 1, 1, 50);
                ImGui::DragInt("Max samples", &renderConfiguration.raysPerPixel, 1, 1, 100);
                
                if (ImGui::Checkbox("Accumulate", &accumulate))
                {
                    renderConfiguration.accumulate = accumulate ? 1 : 0;
                }

                if (ImGui::Button("Reset"))
                {
                    reset = true;
                    renderConfiguration.frameIndex = 1;
                };

                ImGui::End();

                if (renderConfiguration.accumulate == 1)
                {
                    renderConfiguration.frameIndex++;
                }

                ImGui::End();
            
			    ImGui::Render();

			    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			    {
				    ImGui::UpdatePlatformWindows();
				    ImGui::RenderPlatformWindowsDefault();
			    }

			    m_Renderer->EndFrame();

                m_LastRenderTime = timer.ElapsedTime();
            }



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