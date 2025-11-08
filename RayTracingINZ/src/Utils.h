#pragma once

#include <Windows.h>
#include <iostream>
#include <string>

namespace App {


#define CHECK(hr) errorBox(hr, __LINE__, __FILE__);

    inline void errorBox(HRESULT hr, int lineNumber, const char* fileName)
    {

        LPCSTR messageBuffer;
        if (hr == S_OK) return;
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            hr,
            0,
            (LPSTR)&messageBuffer,
            0,
            nullptr);
        if (size > 0)
        {
            char stringBuffer[512];
            snprintf(stringBuffer, sizeof(stringBuffer), "File: %s\nLine: %d\nError code: 0x%X\nError message:%s\n", fileName, lineNumber, hr, messageBuffer);
            MessageBoxA(0, stringBuffer, "ERROR", MB_ICONERROR | MB_OK);
            LocalFree(&stringBuffer);
        }
        else
        {
            MessageBoxA(0, "ERROR", "Failed to retrive the error from HRESULT value", MB_ICONERROR | MB_OK);
        }
        LocalFree(&messageBuffer);
        exit(EXIT_FAILURE);
    }

    //TODO: Moze zrob konsole? 

}