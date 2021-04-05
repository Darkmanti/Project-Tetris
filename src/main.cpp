#include <Windows.h>
#include "win_platform.h"
#include "math.h"
#include "debug_console.h"
#include <Xinput.h>

//init window on windows
struct Win32__Bitmap_Offscreen_Buffer
{
	BITMAPINFO info;
	void* memory;
	int width;
	int height;
	int pitch;
};

struct Win32_Window_Dimension
{
	int width;
	int height;
};

// XInputGetState declare the stubs functions
typedef DWORD (XInputGetStateProc)(DWORD dwUserIndex, XINPUT_STATE* pState);
DWORD XInputGetStateStub(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	return 0;
}
XInputGetStateProc* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// XInputSetState declare the stubs functions
typedef DWORD (XInputSetStateProc)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
DWORD XInputSetStateStub(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
{
	return 0;
}
XInputSetStateProc* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

void Win32LoadXInput()
{
	HMODULE hXInputLibrary = LoadLibraryW(L"Xinput1_4.dll");
	if (hXInputLibrary)
	{
		XInputGetState = (XInputGetStateProc*)GetProcAddress(hXInputLibrary, "XInputGetState");
		XInputSetState = (XInputSetStateProc*)GetProcAddress(hXInputLibrary, "XInputSetState");
	}
	else
	{
		MessageBoxW(NULL, L"Cannot load Xinput1_4.dll", L"error", MB_OK);
	}
}


HWND hMainWnd = {};
// Should move this variable to main loop
MSG msg = {};
Win32__Bitmap_Offscreen_Buffer backbuffer;

Win32_Window_Dimension Win32GetWindowDimension(HWND hWnd)
{
	RECT clientRect = {};
	GetClientRect(hWnd, &clientRect);
	Win32_Window_Dimension rect = {};
	rect.width = clientRect.right - clientRect.left;
	rect.height = clientRect.bottom - clientRect.top;

	return rect;
}

void RenderWeirdGradient(Win32__Bitmap_Offscreen_Buffer* buffer, int xOffset, int yOffset)
{
	u8* row = (u8*)buffer->memory;
	for (int y = 0; y < buffer->height; ++y)
	{
		u32* pixel = (u32*)row;
		for (int x = 0; x < buffer->width; ++x)
		{
			u8 red = 0;
			u8 green = (y + yOffset);
			u8 blue = (x + xOffset);
			u8 reserv = 0;

			*pixel++ = (blue | (green << 8) | (red << 16) | (reserv << 24));
		}

		row += buffer->pitch;
	}
}

// Resize window rect
void Win32ResizeDIBSection(Win32__Bitmap_Offscreen_Buffer* buffer, int width, int height)
{
	if (buffer->memory)
	{
		VirtualFree(buffer->memory, NULL, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;

	int bytesPerPixel = buffer->info.bmiHeader.biBitCount / 8;
	int bitmapMemorySize = (buffer->width * buffer->height) * bytesPerPixel;
	buffer->memory = VirtualAlloc(NULL, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	buffer->pitch = buffer->width * bytesPerPixel;
}

void Win32DisplayBufferInWindow(Win32__Bitmap_Offscreen_Buffer* buffer, HDC deviceContext, int windowWidth, int windowHeight)
{
	StretchDIBits(deviceContext,
				  0, 0, buffer->width, buffer->height,
				  0, 0, windowWidth, windowHeight,
				  buffer->memory,
				  &buffer->info,
				  DIB_RGB_COLORS,
				  SRCCOPY);
}


void Win32ProcessCmdLineArguments(int numArgs, LPWSTR* commandLineArray)
{
	// TODO: maybe optimize it
	// do something with recieve command line arguments
	for (int i = 0; i < numArgs; i++)
	{
		if (wcscmp(commandLineArray[i], L"cmd") == 0)
		{
			con::InitConsole();
		}
	}
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
			Win32_Window_Dimension dimension = Win32GetWindowDimension(hWnd);
			Win32ResizeDIBSection(&backbuffer, dimension.width, dimension.height);
		} break;

		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(NULL);
			}
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
	// num args of CMDLine
	int numArgs = 0;
	// getting an array of CMD parameters. [0] is always absolute .exe file name
	LPWSTR* commandLineArray = CommandLineToArgvW(GetCommandLineW(), &numArgs);

	// process cmd line param
	Win32ProcessCmdLineArguments(numArgs, commandLineArray);

	// TODO: check for OS version
	// set DPI awarness context
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// load icon
	HICON icon = (HICON)LoadImageW(NULL, L"..//res//icon.ico", IMAGE_ICON, NULL, NULL, LR_LOADFROMFILE | LR_DEFAULTSIZE);

	// Load XInput function
	Win32LoadXInput();

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

	int xOffset = 0;
	int yOffset = 0;

	while (true)
	{
		// maybe place MSG in while loop before PeekMessage()
		if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		// Implemented some control
		for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++)
		{
			XINPUT_STATE controllerState;
			ZeroMemory(&controllerState, sizeof(XINPUT_STATE));

			// Check for gamepad plugged in
			if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
			{
				// This Controller is connected
				//con::Outf(L"%ul contorller is connected\n", controllerIndex);
				XINPUT_GAMEPAD* pad = &controllerState.Gamepad;
				bool DPAD_UP =			(pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
				bool DPAD_DOWN =		(pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
				bool DPAD_LEFT =		(pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
				bool DPAD_RIGHT =		(pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
				bool START =			(pad->wButtons & XINPUT_GAMEPAD_START);
				bool BACK =				(pad->wButtons & XINPUT_GAMEPAD_BACK);
				bool LEFT_THUMB =		(pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
				bool RIGHT_THUMB =		(pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
				bool LEFT_SHOULDER =	(pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
				bool RIGHT_SHOULDER =	(pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
				bool A =				(pad->wButtons & XINPUT_GAMEPAD_A);
				bool B =				(pad->wButtons & XINPUT_GAMEPAD_B);
				bool X =				(pad->wButtons & XINPUT_GAMEPAD_X);
				bool Y =				(pad->wButtons & XINPUT_GAMEPAD_Y);

				BYTE  bLeftTrigger = pad->bLeftTrigger;
				BYTE  bRightTrigger = pad->bRightTrigger;
				SHORT sThumbLX = pad->sThumbLX;
				SHORT sThumbLY = pad->sThumbLY;
				SHORT sThumbRX = pad->sThumbRX;
				SHORT sThumbRY = pad->sThumbRY;

				if (sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
				{
					float relativeValue = (float)(sThumbLX - (-32'768)) / (float)65'535;
					int scaledValue = relativeValue * 30 - 15;
					xOffset += scaledValue;
				}
				if (sThumbLY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || sThumbLY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
				{
					float relativeValue = (float)(sThumbLY - (-32'768)) / (float)65'535;
					int scaledValue = relativeValue * 30 - 15;
					yOffset -= scaledValue;
				}

				/*
				XINPUT_VIBRATION vibration;
				ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
				vibration.wRightMotorSpeed = 60000; // use any value between 0-65535 here
				vibration.wLeftMotorSpeed = 60000; // use any value between 0-65535 here
				XInputSetState(controllerIndex, &vibration);
				*/
			}
			else
			{
				// This Controller is not connected
				//con::Outf(L"%ul contorller is not connected\n", controllerIndex);
			}
		}

		// Render some graphics
		Win32_Window_Dimension dimension = Win32GetWindowDimension(hMainWnd);
		RenderWeirdGradient(&backbuffer, xOffset, yOffset);
		Win32DisplayBufferInWindow(&backbuffer, deviceContext, dimension.width, dimension.height);
		//xOffset++;
		//yOffset++;
	}

	// init other thing

	// run the game

	return 0;
}