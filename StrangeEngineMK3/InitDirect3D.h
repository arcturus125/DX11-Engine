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
void CreateRenderTargetView();
bool CreateDepthBuffer();
void BindViewsToOutputMergerStage();
void SetViewport();


// run-time functions

int Run();
void CalculateFrameStats();
void DrawScene();
void OnResize();
void OnMouseDown(WPARAM btnState, int x, int y);
void OnMouseUp(WPARAM btnState, int x, int y);
void OnMouseMove(WPARAM btnState, int x, int y);


