#include "AppWindow.h"

LRESULT CALLBACK App::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	{
		OutputDebugStringA("WM_DESTROY received!\n");
		PostQuitMessage(0);
		break;
	}

	}

	return DefWindowProc(window, message, wParam, lParam);
}

HWND App::CreateWindowApp(const WindowSpecification& spec)
{
	const wchar_t* className = L"RTWINDOW";

	HMODULE hInstane = GetModuleHandle(nullptr);

	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_OWNDC;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstane;
	windowClass.hIcon = nullptr;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.hbrBackground = nullptr;
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = className;
	windowClass.hIconSm = nullptr;

	RegisterClassEx(&windowClass);

	HWND window = CreateWindowEx(
		0, className, L"RayTracing", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, spec.width, spec.height,
		nullptr, nullptr, hInstane, nullptr
	);

	if (window == nullptr)
	{
		return nullptr;
	}

	return window;
}
