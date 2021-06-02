/*
	TODO: this list (this list may supplemented)
	yeah, this list is copy from HandMadeHero

	- Saving game location
	- Getting a gnadle to our own executable file
	- Asset loading path
	- Threading
	- Raw Input
	- Sleep/TimeBeginPeriod
	- ClipCursor() (for multimonitor support)
	- Fullscreen support
	- WM_SETCURSOR (control cursor visibility)
	- QueryCancelAutoplay
	- WM_ACTIVATEAPP (for when we are not the active application)
	- Blit speed improvements (BitBlt)
	- Hardware acceleration (DirectX)
	- GetKeyboardLayout (for international keyboard support layout)

	
*/

// Multiplayer
// Must include before any WinAPI header
#include <winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_PORT "15621"

// User files
#include "tetris.h"
#include "multiplayer.h"
#include "math.h"
#include "debug_console.h"
#include "sound_file_loader.h"
#include "font_proccesing.h"

// WinAPI
#include <Windows.h>

// Gamepad Input
#include <Xinput.h>

// sound engine three variants
// Direct Sound some deprecated engine
#include <dsound.h>

// XAudio2 hopes are pinned on this new engine.
#include <xaudio2.h>

#define MILLI_SECOND(value) (value * 1'000)
#define MICRO_SECOND(value) (value * 1'000'000)

//init window on windows
struct Win32_Bitmap_Offscreen_Buffer
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
	u32 runningSampleIndex;
	int bytesPerSample;
	int secondaryBufferSize;
	f32 tSine;
	int latencySampleCount;
};

struct Win32_XAudio2_Settings
{
	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasterVoice;
};

struct Win32_Socket_Info
{
	WSADATA wsaData;
	SOCKET socket;
	SOCKET clientSocket;
	addrinfo* addrinfo;
};

struct Win32_Recieve_Data
{
	SOCKET id;
	MPRecieveInfo* info;
};

struct Win32_Send_Data
{
	SOCKET id;
	MPSendInfo* info;
};

// some global variable
HWND hMainWnd = {};
// TODO: Should move this variable to main loop
MSG msg = {};
Win32_Bitmap_Offscreen_Buffer backbuffer = {};

// time stamps
LARGE_INTEGER perfFrequency = {};

// Direct Sound
LPDIRECTSOUNDBUFFER secondaryBuffer;

__int64 GetTimeStampMicroSecond()
{
	LARGE_INTEGER time = {};
	QueryPerformanceCounter(&time);
	return ((MICRO_SECOND(time.QuadPart) / perfFrequency.QuadPart));
}

__int64 GetTimeStampMilliSecond()
{
	LARGE_INTEGER time = {};
	QueryPerformanceCounter(&time);
	return ((MILLI_SECOND(time.QuadPart) / perfFrequency.QuadPart));
}

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
						//con::Outf(L"Primary buffer format was set\n");
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
				//con::Outf(L"Secondary buffer created successfully\n");
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

void Win32ClearBuffer(Win32_Sound_Output* soundOutput)
{
	void* region1;
	DWORD region1Size;
	void* region2;
	DWORD region2Size;
	if (SUCCEEDED(secondaryBuffer->Lock(0, soundOutput->secondaryBufferSize, &region1, &region1Size, &region2, &region2Size, NULL)))
	{
		u8* destSample = (u8*)region1;
		for (DWORD byteIndex = 0; byteIndex < region1Size; byteIndex++)
		{
			*destSample++ = 0;
		}

		destSample = (u8*)region2;
		for (DWORD byteIndex = 0; byteIndex < region2Size; byteIndex++)
		{
			*destSample++ = 0;
		}

		secondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
	}
}

void Win32FillSoundBuffer(Win32_Sound_Output* soundOutput, DWORD bytesToLock, DWORD bytesToWrite, Game_Sound_Output_Buffer* sourceBuffer)
{
	void* region1;
	DWORD region1Size;
	void* region2;
	DWORD region2Size;

	if (SUCCEEDED(secondaryBuffer->Lock(bytesToLock, bytesToWrite, &region1, &region1Size, &region2, &region2Size, NULL)))
	{
		DWORD region1SampleCount = region1Size / soundOutput->bytesPerSample;
		i16* destSample = (i16*)region1;
		i16* sourceSample = sourceBuffer->samples;
		for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; sampleIndex++)
		{
			*destSample++ = *sourceSample++;
			*destSample++ = *sourceSample++;
			++soundOutput->runningSampleIndex;
		}

		DWORD region2SampleCount = region2Size / soundOutput->bytesPerSample;
		destSample = (i16*)region2;
		for (DWORD sampleIndex = 0; sampleIndex < region2SampleCount; sampleIndex++)
		{
			*destSample++ = *sourceSample++;
			*destSample++ = *sourceSample++;
			++soundOutput->runningSampleIndex;
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

// Resize window rect
void Win32ResizeDIBSection(Win32_Bitmap_Offscreen_Buffer* buffer, int width, int height)
{
	if (buffer->memory)
	{
		VirtualFree(buffer->memory, NULL, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = buffer->height; // "-" flipping of vertical
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;

	int bytesPerPixel = buffer->info.bmiHeader.biBitCount / 8;
	int bitmapMemorySize = (buffer->width * buffer->height) * bytesPerPixel;
	buffer->memory = VirtualAlloc(NULL, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	buffer->pitch = buffer->width * bytesPerPixel;
}

void Win32DisplayBufferInWindow(Win32_Bitmap_Offscreen_Buffer* buffer, HDC deviceContext, int windowWidth, int windowHeight)
{
	StretchDIBits(deviceContext,
				  0, 0, buffer->width, buffer->height,
				  0, 0, windowWidth, windowHeight,
				  buffer->memory,
				  &buffer->info,
				  DIB_RGB_COLORS,
				  SRCCOPY);
}

int Win32SetStateWithDialogBox()
{
	int msgboxID = MessageBox(
		NULL,
		L"Single? Join? Host?",
		L"Choose game state",
		MB_ICONQUESTION | MB_CANCELTRYCONTINUE | MB_DEFBUTTON1
	);

	switch (msgboxID)
	{
		case IDCANCEL:
		{
			msgboxID = 0;
		} break;
		case IDTRYAGAIN:
		{
			msgboxID = 1;
		} break;
		case IDCONTINUE:
		{
			msgboxID = 2;
		} break;
	}

	return msgboxID;
}

void Win32InitWSADATA(Win32_Socket_Info* info)
{
	WSADATA* wsaData = &info->wsaData;

	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), wsaData);
	if (iResult != 0)
	{
		con::Outf(L"WSAStartup failed: %i32\n", iResult);
	}

	if (LOBYTE(wsaData->wVersion) != 2 || HIBYTE(wsaData->wVersion) != 2)
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		con::Outf(L"Could not find a usable version of Winsock.dll\n");
		WSACleanup();
	}
	else
	{
		//con::Outf(L"The Winsock 2.2 dll was found okay\n");
	}
}

void Win32CreateServerSocket(Win32_Socket_Info* info)
{
	addrinfo hints;
	addrinfo* result = info->addrinfo;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	// Resolve the local address and port to be used by the server
	int iResult;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		con::Outf(L"getaddrinfo failed: %i32\n", iResult);
		WSACleanup();
	}

	SOCKET* ListenSocket = &info->socket;
	*ListenSocket = INVALID_SOCKET;

	// Create a SOCKET for the server to listen for client connections
	*ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (*ListenSocket == INVALID_SOCKET)
	{
		con::Outf(L"Error at socket(): %i32\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
	}

	// Setup the TCP listening socket
	iResult = bind(*ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		con::Outf(L"bind failed with error: %i32\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(*ListenSocket);
		WSACleanup();
	}

	// Once the bind function is called, the address information returned by the getaddrinfo function is no longer needed. 
	// The freeaddrinfo function is called to free the memory allocated by the getaddrinfo function for this address information.
	//freeaddrinfo(result);

	if (listen(*ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		con::Outf(L"Listen failed with error: %i32\n", WSAGetLastError());
		closesocket(*ListenSocket);
		WSACleanup();
	}

	con::Outf(L"Server has been running\n");
}

void Win32ListenConnectionToServer(Win32_Socket_Info* info)
{
	// Accept a client socket
	con::Outf(L"Start listening to connect to the server\n");
	info->clientSocket = accept(info->socket, NULL, NULL);
	if (info->clientSocket == INVALID_SOCKET)
	{
		con::Outf(L"accept failed: %i32\n", WSAGetLastError());
		closesocket(info->socket);
		return;
	}

	connectionEstablished = true;
	con::Outf(L"Connect to client established\n");
}

void Win32CreateClientSocket(Win32_Socket_Info* info, IPStrucrute* ip)
{
	struct addrinfo* result = info->addrinfo, * ptr = NULL, hints;
	int iResult;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char string[32] = "";
	wcstombs(string, ip->ipString, 32);

	// Resolve the server address and port
	if (strlen(string) > 9)
	{
		iResult = getaddrinfo(string, DEFAULT_PORT, &hints, &result);
	}
	if (iResult != 0)
	{
		con::Outf(L"getaddrinfo failed: %i32\n", iResult);
		freeaddrinfo(result);
		return;
	}

	SOCKET* ConnectSocket = &info->socket;
	*ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by the call to getaddrinfo
	ptr = result;

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		*ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (*ConnectSocket == INVALID_SOCKET)
		{
			con::Outf(L"socket failed with error: %i32\n", WSAGetLastError());
			freeaddrinfo(result);
			return;
		}

		// Connect to server.
		iResult = connect(*ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(*ConnectSocket);
			*ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	if (*ConnectSocket == INVALID_SOCKET)
	{
		con::Outf(L"Unable to connect to server!\n");
		freeaddrinfo(result);
		return;
	}

	connectionEstablished = true;
	con::Outf(L"Connection to server established\n");

	info->clientSocket = *ConnectSocket;
}

void Win32RecieveInfo(Win32_Recieve_Data* data)
{
	int iResult = 0;
	do
	{
		//_mm_pause();
		char recvbuf[DEFAULT_BUFLEN] = "";
		iResult = recv(data->id, recvbuf, DEFAULT_BUFLEN, 0);
		//con::Outf(L"%i32 bytes recieve\n", iResult);

		if (iResult > 0)
		{
			BufferToGameInfo(data->info, recvbuf);
		}
		else if (iResult == 0)
		{
			con::Outf(L"Connection closed\n");
			// ?????????DISCONNECT????????
		}
		else
		{
			con::Outf(L"recv failed: %i32\n", WSAGetLastError());
			// DISCONNECT
		}
	} while (iResult > 0);
}

void Win32SendGameField(Win32_Send_Data* data)
{
	char sendbuf[DEFAULT_BUFLEN] = "";
	GameInfoToBuffer(data->info, sendbuf);
	int iSendResult = send(data->id, sendbuf, DEFAULT_BUFLEN, 0);
	//con::Outf(L"%i32 bytes send\n", iSendResult);
	if (iSendResult == SOCKET_ERROR)
	{
		con::Outf(L"send failed: %i32\n", WSAGetLastError());
		closesocket(data->id);
		allowToSend = false;
		// DISCONNECT
	}
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

	// set the performance frequency
	QueryPerformanceFrequency(&perfFrequency);

	// set DPI awarness context
	if (IsValidDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
	{
		SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	}
	else if (IsValidDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE))
	{
		SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
	}
	else
	{
		SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
	}
	
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

	// Set Tetris generation seed
	// TODO: Move this from here
	srand((u32)GetTimeStampMilliSecond());

	HDC deviceContext = GetDC(hMainWnd);

	// XAudio2
	Win32_XAudio2_Settings xAudio2Output = {};
	WAVEFORMATEXTENSIBLE wfx = {0};
	XAUDIO2_BUFFER buffer = {0};
	Win32InitXAudio2(&xAudio2Output);
	sfl::Win32LoadSoundFromFileWAV(L"..//res//blade.wav", &wfx, &buffer);

	// Playing XAudio2 sound
	IXAudio2SourceVoice* pSourceVoice;
	xAudio2Output.pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx);
	pSourceVoice->SubmitSourceBuffer(&buffer);
	pSourceVoice->Start(0);

	// Direct Sound testing
	Win32_Sound_Output soundOutput = {};

	soundOutput.samplesPerSecond = 48000;
	soundOutput.bytesPerSample = sizeof(i16) * 2;
	soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond * soundOutput.bytesPerSample;
	soundOutput.latencySampleCount = soundOutput.samplesPerSecond / 15;

	Win32InitDSound(hMainWnd, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
	Win32ClearBuffer(&soundOutput);
	secondaryBuffer->Play(NULL, NULL, DSBPLAY_LOOPING);

	i16* samples = (i16*)VirtualAlloc(NULL, soundOutput.secondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	// TODO: GameState
	Tetris_Game_Field rivalField = {};
	MPRecieveInfo rivalInfo = {};
	rivalInfo.field = &rivalField;
	Win32_Recieve_Data recieveData = {};
	recieveData.info = &rivalInfo;
	MPSendInfo myInfo = {};
	Win32_Send_Data sendData = {};
	sendData.info = &myInfo;
	multiplayerState = Win32SetStateWithDialogBox();
	Win32_Socket_Info socketInfo;
	if (multiplayerState > 0)
	{
		Win32InitWSADATA(&socketInfo);
		InitGameField(&rivalField);

		if (multiplayerState == 1) // client
		{
			// Nothing
		}
		else if (multiplayerState == 2) // server
		{
			Win32CreateServerSocket(&socketInfo);
		}
	}

	// Fonts
	// TODO: this!!!
	InitFont(&font, 80, 1200, L"..//res//fonts//OpenSans-Semibold.ttf", 32, 2048, 2048);

	i64 currentFrame = 0;

	Game_Memory gameMemory = {};
	gameMemory.permanentStorageSize = Megabytes(64);
	gameMemory.transientStorageSize = Gigabytes(1);
	u64 totalSize = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
	gameMemory.permanentStorage = VirtualAlloc(NULL, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	gameMemory.transientStorage = (u8*)gameMemory.permanentStorage + gameMemory.permanentStorageSize;

	// TODO: Logging about lack of memory and shutdown the process
	
	// run the game
	while (true)
	{
		Game_Input input = {};

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

		// Implemented some XInput control
		for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++)
		{
			XINPUT_STATE controllerState;
			ZeroMemory(&controllerState, sizeof(XINPUT_STATE));

			// Check for gamepad plugged in
			if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
			{
				// This Controller is connected
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

				// TODO: fill the input structure

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
			}
		}

		// Direct Sound testing
		DWORD playCursorPosition = 0;
		DWORD writeCursorPosition = 0;
		DWORD bytesToLock = 0;
		DWORD targetCursor = 0;
		DWORD bytesToWrite = 0;
		bool soundIsValid = false;
		if (SUCCEEDED(secondaryBuffer->GetCurrentPosition(&playCursorPosition, &writeCursorPosition)))
		{
			bytesToLock = (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.secondaryBufferSize;
			targetCursor = ((playCursorPosition + (soundOutput.latencySampleCount * soundOutput.bytesPerSample)) % soundOutput.secondaryBufferSize);
			if (bytesToLock > targetCursor)
			{
				bytesToWrite = soundOutput.secondaryBufferSize - bytesToLock;
				bytesToWrite += targetCursor;
			}
			else
			{
				bytesToWrite = targetCursor - bytesToLock;
			}
			soundIsValid = true;
		}

		Game_Sound_Output_Buffer soundBuffer = {};
		soundBuffer.samplesPerSecond = soundOutput.samplesPerSecond;
		soundBuffer.sampleCount = bytesToWrite / soundOutput.bytesPerSample;
		soundBuffer.samples = samples;

		if (soundIsValid)
		{
			Win32FillSoundBuffer(&soundOutput, bytesToLock, bytesToWrite, &soundBuffer);
		}

		if (multiplayerState == 2 && listenConnectToServer)
		{
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Win32ListenConnectionToServer, &socketInfo, NULL, NULL);
			listenConnectToServer = false;
		}

		if (!connectionEstablished && tryToConnectClient)
		{
			con::Outf(L"Happen\n");
			// NOTE: BLOCK CALL
			Win32CreateClientSocket(&socketInfo, &ipStructure);
			tryToConnectClient = false;
			if (connectionEstablished)
			{
				ClearInput(&ipStructure);
				//DisableInput
			}
		}

		if (connectionEstablished)
		{
			recieveData.id = socketInfo.clientSocket;
			sendData.info->field = &hostGameState.field;
			sendData.info->currentScore = &hostGameState.currentScore;
			sendData.info->maxScore = &hostGameState.maxScore;
			sendData.id = socketInfo.clientSocket;
			connectionEstablished = false;
			allowToCreateRecieveThread = true;
			allowToSend = true;
			runMPGame = true;
		}

		if (allowToCreateRecieveThread)
		{
			allowToCreateRecieveThread = false;
			// recieve info Fill rivalField 
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Win32RecieveInfo, &recieveData, NULL, NULL);
		}

		if (allowToSend)
		{
			// send info Filling myField;
			Win32SendGameField(&sendData);
		}

		// Render some graphics
		Win32_Window_Dimension dimension = Win32GetWindowDimension(hMainWnd);

		Game_Bitmap_Offscreen_Buffer buffer = {};
		buffer.memory = backbuffer.memory;
		buffer.width = backbuffer.width;
		buffer.height = backbuffer.height;
		buffer.pitch = backbuffer.pitch;
		GameUpdateAndRender(&gameMemory, &input, &buffer, &soundBuffer, currentFrame, &rivalInfo);

		Win32DisplayBufferInWindow(&backbuffer, deviceContext, dimension.width, dimension.height);

		currentFrame = GetTimeStampMilliSecond();
	}

	return 0;
}