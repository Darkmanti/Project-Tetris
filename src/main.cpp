#include <Windows.h>
#include "win_platform.h"
#include "Math.h"

//init window on windows

BITMAPINFO bitmapInfo = {};
void* bitmapMemory;
int bitmapWidth;
int bitmapHeight;

HWND hMainWnd = {};
MSG msg = {};

void RenderWeirdGradient(int xOffset, int yOffset)
{
	int bytesPerPixel = bitmapInfo.bmiHeader.biBitCount / 8;
	int pitch = bitmapWidth * bytesPerPixel;
	u8* row = (u8*)bitmapMemory;
	for (int y = 0; y < bitmapHeight; ++y)
	{
		u32* pixel = (u32*)row;
		for (int x = 0; x < bitmapWidth; ++x)
		{
			u8 red = 0;
			u8 green = (y + yOffset);
			u8 blue = (x + xOffset);
			u8 reserv = 0;

			*pixel++ = (blue | (green << 8) | (red << 16) | (reserv << 24));
		}

		row += pitch;
	}
}

// Resize window rect
void Win32ResizeDIBSection(int width, int height)
{
	if (bitmapMemory)
	{
		VirtualFree(bitmapMemory, NULL, MEM_RELEASE);
	}

	bitmapWidth = width;
	bitmapHeight = height;

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = bitmapWidth;
	bitmapInfo.bmiHeader.biHeight = -bitmapHeight;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	int bytesPerPixel = bitmapInfo.bmiHeader.biBitCount / 8;
	int bitmapMemorySize = (bitmapWidth * bitmapHeight) * bytesPerPixel;
	bitmapMemory = VirtualAlloc(NULL, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

void Win32UpdateWindow(HDC deviceContext, RECT *windowRect)
{
	int windowWidth = windowRect->right - windowRect->left;
	int windowHeight = windowRect->bottom - windowRect->top;
	StretchDIBits(deviceContext,
				  0, 0, bitmapWidth, bitmapHeight,
				  0, 0, windowWidth, windowHeight,
				  bitmapMemory,
				  &bitmapInfo,
				  DIB_RGB_COLORS,
				  SRCCOPY);
}

LRESULT CALLBACK Win32MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
			// send quit message in main cycle
			PostQuitMessage(NULL);
		} break;

		case WM_CLOSE:
		{
			// warning about the possible closing of the window
			DestroyWindow(hWnd);
		} break;

		case WM_ACTIVATE:
		{
			// activate and deactivate window depend on wParam
		} break;

		case WM_SIZE:
		{
			// handle window resizing. Also calls when window created
			RECT clientRect = {};
			GetClientRect(hWnd, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			Win32ResizeDIBSection(width, height);
		} break;

		default:
		{
			return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		} break;
	}
	return 0;
}

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
	HICON icon = (HICON)LoadImageW(NULL, L"..//res//icon.ico", IMAGE_ICON, NULL, NULL, LR_LOADFROMFILE | LR_DEFAULTSIZE);

	WNDCLASSEXW mainWndClass = {
		sizeof(WNDCLASSEXW),				// UINT      cbSize;
		CS_OWNDC | CS_HREDRAW | CS_VREDRAW,	// UINT      style;
		Win32MainWndProc,					// WNDPROC   lpfnWndProc;
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

	HDC deviceContext = GetDC(hMainWnd);
	RECT clientRect = {};

	int xOffset = 0;
	int yOffset = 0;

	while (true)
	{
		if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		// Render some graphics
		RenderWeirdGradient(xOffset, yOffset);
		GetClientRect(hMainWnd, &clientRect);
		Win32UpdateWindow(deviceContext, &clientRect);
		xOffset++;
		yOffset++;
	}

	// init other thing

	// run the game

	return 0;
}