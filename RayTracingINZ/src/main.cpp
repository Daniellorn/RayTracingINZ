#include <windows.h>

#include "Application.h"
#include <iostream>
#include <filesystem>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	App::Application app;
	app.Run();

	return 0;
}