#pragma once
#include <d3d11.h>
#include <dxerr.h>

// message handlers
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// init functions
void SetDefaults(HINSTANCE hInstance);
bool InitMainWindow();
bool CreateDeviceAndContext();
bool Check4xMSAAQualitySupport();
bool DescribeSwapChain();


// run-time functions
void OnResize();
void OnMouseDown(WPARAM btnState, int x, int y);
void OnMouseUp(WPARAM btnState, int x, int y);
void OnMouseMove(WPARAM btnState, int x, int y);


