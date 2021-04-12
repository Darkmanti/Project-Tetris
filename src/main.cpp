#include <Windows.h>
#include "win_platform.h"
#include "math.h"
#include "debug_console.h"
#include <Xinput.h>

#include "sound_file_loader.h"
// sound engine three variants
// Direct Sound some deprecated engine
#include <dsound.h>

// XAudio2 hopes are pinned on this new engine.
#include <xaudio2.h>

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

struct Win32_Sound_Output
{
	int samplesPerSecond;
	int toneHz;
	i16 toneVolume;
	u32 runningSampleIndex;
	int wavePeriod;
	int bytesPerSample;
	int secondaryBufferSize;
};

struct Win32_XAudio2_Settings
{
	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasterVoice;
};

// some global variable
HWND hMainWnd = {};
// Should move this variable to main loop
MSG msg = {};
Win32__Bitmap_Offscreen_Buffer backbuffer;

// Direct Sound
LPDIRECTSOUNDBUFFER secondaryBuffer;

// XInput definitions
// XInputGetState declare the stubs functions
typedef DWORD (XInputGetStateProc)(DWORD dwUserIndex, XINPUT_STATE* pState);
DWORD XInputGetStateStub(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
XInputGetStateProc* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// XInputSetState declare the stubs functions
typedef DWORD (XInputSetStateProc)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
DWORD XInputSetStateStub(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
XInputSetStateProc* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// TODO: add some functions in this case
void Win32LoadXInput()
{
	HMODULE hXInputLibrary = LoadLibraryW(L"Xinput1_4.dll");
	if (!hXInputLibrary)
	{
		hXInputLibrary = LoadLibraryW(L"Xinput1_3.dll");
	}
	if (hXInputLibrary)
	{
		XInputGetState = (XInputGetStateProc*)GetProcAddress(hXInputLibrary, "XInputGetState");
		if (!XInputGetState) { XInputGetState = XInputGetStateStub; }

		XInputSetState = (XInputSetStateProc*)GetProcAddress(hXInputLibrary, "XInputSetState");
		if (!XInputSetState) { XInputSetState = XInputSetStateStub; }
	}
	else
	{
		MessageBoxW(NULL, L"Cannot load Xinput1_4.dll or Xinput1_3.dll", L"error", MB_OK);
	}
}

// DurectSound definitions
typedef HRESULT(DirectSoundCreate8Proc)(LPCGUID lpcGuidDevice, LPDIRECTSOUND8* ppDS8, LPUNKNOWN pUnkOuter);

// Direct Sound functions
void Win32InitDSound(HWND window, DWORD samplesPerSecond, DWORD bufferSize)
{
	// Load the library
	HMODULE hDSoundLibrary = LoadLibraryW(L"dsound.dll");

	if (hDSoundLibrary)
	{
		// Get a DirectSound object
		DirectSoundCreate8Proc* DirectSoundCreate8 = (DirectSoundCreate8Proc*)GetProcAddress(hDSoundLibrary, "DirectSoundCreate8");

		LPDIRECTSOUND8 directSound;
		if (DirectSoundCreate8 && SUCCEEDED(DirectSoundCreate8(NULL, &directSound, NULL)))
		{
			// Create wave format
			WORD nChannels = 2;
			WORD wBitsPerSample = 16;
			WORD nBlockAlign = (wBitsPerSample * nChannels) / 8;
			WAVEFORMATEX waveFormat = {
				WAVE_FORMAT_PCM,				//For one - or two - channel PCM data, this value should be
				nChannels,						// Number of channels in the waveform-audio data
				samplesPerSecond,				// Sample rate, in samples per second (hertz)
				samplesPerSecond * nBlockAlign,	// Required average data-transfer rate, in bytes per second, for the format tag product of nSamplesPerSec and nBlockAlign
				nBlockAlign,					// Block alignment, in bytes. nChannels and wBitsPerSample divided by 8
				wBitsPerSample,					// Bits per sample for the wFormatTag format type. If WAVE_FORMAT_PCM then 8 or 16
				0								// For WAVE_FORMAT_PCM formats (and only WAVE_FORMAT_PCM formats), this member is ignored
			};

			if (SUCCEEDED(directSound->SetCooperativeLevel(window, DSSCL_PRIORITY))) // try NORMAL
			{
				DSBUFFERDESC bufferDescription = {
					sizeof(bufferDescription),						// Size of the structure, in bytes
					DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME,		// Flags specifying the capabilities of the buffer
					NULL,											// his value must be 0 when creating a buffer with the DSBCAPS_PRIMARYBUFFER flag
					NULL,											// Reserved must be 0
					NULL,											// This value must be NULL for primary buffers.
					DS3DALG_DEFAULT									// DSBCAPS_CTRL3D is not set in dwFlags, this member must be GUID_NULL (DS3DALG_DEFAULT)
				};

				// Create a primary buffer
				LPDIRECTSOUNDBUFFER primaryBuffer = {};
				if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, NULL)))
				{
					if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat)))
					{
						// We have finally set the format
						con::Outf(L"Primary buffer format was set\n");
					}
					else
					{
						// TODO: Diagnostic
					}
				}
				else
				{
					// TODO: Diagnostic
				}
			}
			else
			{
				// TODO: Diagnostic
			}

			// Create a secondary buffer
			DSBUFFERDESC bufferDescription = {
					sizeof(bufferDescription),					// Size of the structure, in bytes
					DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN,		// Flags specifying the capabilities of the buffer
					bufferSize,									// his value must be 0 when creating a buffer with the DSBCAPS_PRIMARYBUFFER flag
					NULL,										// Reserved must be 0
					&waveFormat,								// This value must be NULL for primary buffers.
					DS3DALG_DEFAULT								// DSBCAPS_CTRL3D is not set in dwFlags, this member must be GUID_NULL (DS3DALG_DEFAULT)
			};

			if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &secondaryBuffer, NULL)))
			{
				// Start it playing
				con::Outf(L"Secondary buffer created successfully\n");
			}
			else
			{
				// TODO: Diagnostic
			}
		}
		else
		{
			// TODO: Diagnostic
		}
	}
	else
	{
		MessageBoxW(NULL, L"Cannot load dsound.dll", L"error", MB_OK);
	}
}

void Win32FillSoundBuffer(Win32_Sound_Output* soundOutput, DWORD bytesToLock, DWORD bytesToWrite)
{
	void* region1;
	DWORD region1Size;
	void* region2;
	DWORD region2Size;

	if (SUCCEEDED(secondaryBuffer->Lock(bytesToLock, bytesToWrite, &region1, &region1Size, &region2, &region2Size, NULL)))
	{
		i16* sampleOut = (i16*)region1;
		DWORD region1SampleCount = region1Size / soundOutput->bytesPerSample;

		for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; sampleIndex++)
		{
			f32 t = 2.0f * PI_32 * (f32)soundOutput->runningSampleIndex / (f32)soundOutput->wavePeriod;
			f32 sineValue = Sin(t);
			i16 sampleValue = (i16)(sineValue * soundOutput->toneVolume);
			*sampleOut++ = sampleValue;
			*sampleOut++ = sampleValue;
			soundOutput->runningSampleIndex++;
		}

		sampleOut = (i16*)region2;
		DWORD region2SampleCount = region2Size / soundOutput->bytesPerSample;
		for (DWORD sampleIndex = 0; sampleIndex < region2SampleCount; sampleIndex++)
		{
			f32 t = 2.0f * PI_32 * (f32)soundOutput->runningSampleIndex / (f32)soundOutput->wavePeriod;
			f32 sineValue = Sin(t);
			i16 sampleValue = (i16)(sineValue * soundOutput->toneVolume);
			*sampleOut++ = sampleValue;
			*sampleOut++ = sampleValue;
			soundOutput->runningSampleIndex++;
		}

		secondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
	}
}

// XAudio2 functions
void Win32InitXAudio2(Win32_XAudio2_Settings* xAudio2Output)
{
	// TODO: maybe collaps these diagnostic and extended some functions
	// TODO: Check if this function is really needed because it requires some strange library
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		MessageBoxW(NULL, L"failed to initialize COM", L"error", MB_OK);
		return;
	}
	HRESULT errors = XAudio2Create(&xAudio2Output->pXAudio2, NULL, XAUDIO2_DEFAULT_PROCESSOR);
	switch (errors)
	{
		case S_OK:
		{
			// successfull
		} break;
		case XAUDIO2_E_INVALID_CALL:
		{
			MessageBoxW(NULL, L"XAUDIO2_E_INVALID_CALL", L"error", MB_OK);
			return;
		} break;
		case XAUDIO2_E_XMA_DECODER_ERROR:
		{
			MessageBoxW(NULL, L"XAUDIO2_E_XMA_DECODER_ERROR", L"error", MB_OK);
			return;
		} break;
		case XAUDIO2_E_XAPO_CREATION_FAILED:
		{
			MessageBoxW(NULL, L"XAUDIO2_E_XAPO_CREATION_FAILED", L"error", MB_OK);
			return;
		} break;
		case XAUDIO2_E_DEVICE_INVALIDATED:
		{
			MessageBoxW(NULL, L"XAUDIO2_E_DEVICE_INVALIDATED", L"error", MB_OK);
			return;
		} break;
		invalid_default
	}

	if(FAILED((xAudio2Output->pXAudio2)->CreateMasteringVoice(
			&xAudio2Output->pMasterVoice,	// pointer to master voice
			XAUDIO2_DEFAULT_CHANNELS,		// audio chanel. if XAUDIO2_DEFAULT_CHANNELS to try to detect the system speaker configuration setup.
			XAUDIO2_DEFAULT_SAMPLERATE,		// number of samples. if XAUDIO2_DEFAULT_SAMPLERATE being determined by the current platform.
			NULL,							// flags. Always NULL
			NULL,							// Identifier of the device to receive the output audio. NULL causes XAudio2 to select the global default audio device.
			NULL							// Pointer to an XAUDIO2_EFFECT_CHAIN. NULL to use no effects.
			//??							// StreamCategory The audio stream category to use for this mastering voice. DEFAULT
		)))
	{
		MessageBoxW(NULL, L"failed to create mastering voice", L"error", MB_OK);
		return;
	}
}

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
	buffer->info.bmiHeader.biHeight = -buffer->height; // "-" flipping of vertical
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
	for (int i = 1; i < numArgs; i++)
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

		// TODO: maybe this is redundantly
		case WM_SYSKEYDOWN:
		{
			if (wParam == VK_F4)
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

	// init other thing

	HDC deviceContext = GetDC(hMainWnd);

	int xOffset = 0;
	int yOffset = 0;

	// XAudio2
	Win32_XAudio2_Settings xAudio2Output = {};
	WAVEFORMATEXTENSIBLE wfx = {0};
	XAUDIO2_BUFFER buffer = {0};
	Win32InitXAudio2(&xAudio2Output);
	sfl::Win32LoadSoundFromFileWAV(L"../res/blade.wav", &wfx, &buffer);

	// Playing XAudio2 sound
	IXAudio2SourceVoice* pSourceVoice;
	xAudio2Output.pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx);
	pSourceVoice->SubmitSourceBuffer(&buffer);
	pSourceVoice->Start(0);

	// Direct Sound testing
	/*Win32_Sound_Output soundOutput = {};

	soundOutput.samplesPerSecond = 48000;
	soundOutput.toneHz = 256;
	soundOutput.toneVolume = 2000;
	soundOutput.runningSampleIndex = 0;
	soundOutput.wavePeriod = soundOutput.samplesPerSecond / soundOutput.toneHz;
	soundOutput.bytesPerSample = sizeof(i16) * 2;
	soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond * soundOutput.bytesPerSample;

	Win32InitDSound(hMainWnd, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
	Win32FillSoundBuffer(&soundOutput, 0, soundOutput.secondaryBufferSize);

	secondaryBuffer->Play(NULL, NULL, DSBPLAY_LOOPING);*/

	// run the game
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
		
		// Direct Sound testing
		/*DWORD playCursorPosition;
		DWORD writeCursorPosition;
		if (SUCCEEDED(secondaryBuffer->GetCurrentPosition(&playCursorPosition, &writeCursorPosition)))
		{
			DWORD bytesToLock = (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.secondaryBufferSize;
			DWORD bytesToWrite = 0;
			if (bytesToLock > playCursorPosition)
			{
				bytesToWrite = soundOutput.secondaryBufferSize - bytesToLock;
				bytesToWrite += playCursorPosition;
			}
			else
			{
				bytesToWrite = playCursorPosition - bytesToLock;
			}

			Win32FillSoundBuffer(&soundOutput, bytesToLock, bytesToWrite);
		}*/
	}

	return 0;
}