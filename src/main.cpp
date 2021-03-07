#include <Windows.h>
#include "win_platform.h"

//init window
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hMainWnd = {};
MSG msg = {};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// TODO check for OS version
	// set DPI awarness context
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// num args of CMDLine
	int numArgs = 0;
	// getting an array of CMD parameters. [0] is always absolute .exe file name
	CommandLineToArgvW(GetCommandLineW(), &numArgs);

	// load icon
	HICON icon = (HICON)LoadImageW(NULL, L"../res/icon.ico", IMAGE_ICON, NULL, NULL, LR_LOADFROMFILE | LR_DEFAULTSIZE);

	WNDCLASSEXW mainWndClass = {
		sizeof(WNDCLASSEX),					// UINT      cbSize;
		CS_OWNDC | CS_HREDRAW | CS_VREDRAW,	// UINT      style;
		MainWndProc,						// WNDPROC   lpfnWndProc;
		NULL,								// int       cbClsExtra;
		NULL,								// int       cbWndExtra;
		hInstance,							// HINSTANCE hInstance;
		icon,								// HICON     hIcon;
		NULL,								// HCURSOR   hCursor;
		NULL,								// HBRUSH    hbrBackground;
		NULL,								// LPCWSTR   lpszMenuName;
		L"MainWindow",						// LPCWSTR   lpszClassName;
		icon								// HICON     hIconSm
	};

	// Register the new window class and check for creation errors
	if (!RegisterClassEx(&mainWndClass))
	{
		MessageBoxW(NULL, L"Cannot register the main window class", L"error", MB_OK);
		return NULL;
	}
	
	// here set the real window size with AdjustWindowRectEx()


	// create window
	hMainWnd = CreateWindowExW(
		NULL,								// DWORD     dwExStyle,
		mainWndClass.lpszClassName,			// LPCSTR    lpClassName,
		L"Tetris",							// LPCSTR    lpWindowName,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,	// DWORD     dwStyle,
		CW_USEDEFAULT,						// int       X,
		CW_USEDEFAULT,						// int       Y,
		1280,								// int       nWidth,
		720,								// int       nHeight,
		NULL,								// HWND      hWndParent,
		NULL,								// HMENU     hMenu,
		hInstance,							// HINSTANCE hInstance,
		NULL								// LPVOID    lpParam
	);

	// validation of hMainWnd
	if (!hMainWnd)
	{
		MessageBoxW(NULL, L"Cannot create a Main Window", L"error", MB_OK);
		return NULL;
	}

	while (true)
	{
		// try use getmessage
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	// init other thing

	// run the game

	return 0;
}


// init window
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:
		{
			// Window already created
		} break;

		case WM_NCCREATE:
		{
			// Window being create
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		} break;

		case WM_DESTROY:
		{
			PostQuitMessage(NULL);
		} break;

		case WM_CLOSE:
		{
			// warning about the possible closing of the window
			DestroyWindow(hWnd);
		} break;

		case WM_SIZE:
		{
			// handle window resizing
		} break;

		default:
		{
			return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		} break;
	}
	return 0;
}