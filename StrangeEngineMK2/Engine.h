#pragma once


//#include "pch.h"
#include "Scene.h"
#include "Direct3DSetup.h"
#include "Input.h"
#include "Timer.h"
//#include "Common.h"
#include <windows.h>
#include <string>
#include <shellapi.h>


// Forward declarations of functions in this file
__declspec(dllexport) BOOL             InitWindow(HINSTANCE);
__declspec(dllexport) LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


__declspec(dllexport) void AddShaderFolder(std::string path);
__declspec(dllexport) void AddMediaFolder(std::string path);

__declspec(dllexport) int StartEngine(_In_     HINSTANCE hInstance, void (*start)(), void (*update)(float));