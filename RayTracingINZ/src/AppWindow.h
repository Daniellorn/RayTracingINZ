#pragma once

#include <windows.h>
#include <string>

namespace App {

	struct WindowSpecification
	{
		int clientWidth = 1280;
		int clientHeight = 720;
		int windowWidth = 1280;
		int windowHeight = 720;
		const std::wstring title = L"RayTracingApp";
	};

	static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);


	HWND CreateWindowApp(WindowSpecification& spec);
}