#include <windows.h>

#include "Application.h"
#include <iostream>
#include <filesystem>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{

	std::string filepath = std::filesystem::current_path().string();

	App::Application app;
	app.Run();

	return 0;
}