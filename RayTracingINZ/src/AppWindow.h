#pragma once

#include <windows.h>
#include <string>

namespace App {

	struct WindowSpecification
	{
		int width = 1280;
		int height = 720;
		const std::wstring title = L"RayTracingApp";
	};

	static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);


	HWND CreateWindowApp(const WindowSpecification& spec = WindowSpecification());
}