#include "AppWindow.h"

#include "Utils.h"
#include "Renderer.h"
#include <imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK App::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{

	Renderer* renderer = (Renderer*)GetWindowLongPtr(window, GWLP_USERDATA);

	if (ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam))
		return true;

	switch (message)
	{
		case WM_DESTROY:
			OutputDebugStringA("WM_DESTROY received!\n");
			PostQuitMessage(0);
			return 0;
		//case WM_SIZE:
		//{
		//	RECT clientRect = { 0 };
		//	GetClientRect(window, &clientRect);
		//	
		//	int clientWidth = clientRect.right - clientRect.left;
		//	int clientHeight = clientRect.bottom - clientRect.top;
		//	
		//	if (clientWidth > 0 && clientHeight > 0)
		//		renderer->Resize(clientWidth, clientHeight);
		//}break;


	}

	return DefWindowProc(window, message, wParam, lParam);
}

HWND App::CreateWindowApp(WindowSpecification& spec)
{
	const wchar_t* className = L"RTWINDOW";

	HMODULE hInstance = GetModuleHandle(nullptr);

	WNDCLASSEXW windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_OWNDC;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = nullptr;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.hbrBackground = nullptr;
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = className;
	windowClass.hIconSm = nullptr;

	RegisterClassExW(&windowClass);

	uint32_t windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION
		| WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	uint32_t windowExStyle = WS_EX_APPWINDOW;
	   
	uint32_t windowX = 100;
	uint32_t windowY = 100;

	RECT borderRect = { 0, 0, 1280, 720 };
	AdjustWindowRectEx(&borderRect, windowStyle, false, windowExStyle);

	windowX += borderRect.left;
	windowY += borderRect.top;

	spec.windowWidth = borderRect.right - borderRect.left;
	spec.windowHeight = borderRect.bottom - borderRect.top;

	HWND window = CreateWindowExW(
		windowExStyle, className, L"RayTracing", WS_OVERLAPPEDWINDOW,
		windowX, windowY, spec.windowWidth, spec.windowHeight,
		nullptr, nullptr, hInstance, nullptr
	);

	if (window == nullptr)
	{
		return nullptr;
	}

	return window;
}
