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

#define MILLI_SECOND(x) x * 1'000
#define MICRO_SECOND(x) x * 1'000'000

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
Win32__Bitmap_Offscreen_Buffer backbuffer = {};

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
	buffer->info.bmiHeader.biHeight = buffer->height; // "-" flipping of vertical
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

// Tetris realize
// TODO: all of this functions need to optimize
bool keyState[256] = {};

const int gameFieldWidth = 10;
const int gameFieldHeight = 20;
int gameField[gameFieldWidth][gameFieldHeight]{};

i64 lastFrame = 0;
i64 currentFrame = 0;
i64 timeElapsed = 0;
i64 speedMillisecond = 1000;

i64 controlDownOnePressElapsed = 0;
i64 controlDownElapsed = 0;
i64 controlLeftOnePressElapsed = 0;
i64 controlLeftElapsed = 0;
i64 controlRightOnePressElapsed = 0;
i64 controlRightElapsed = 0;

i64 controlOnePressDelay = 450;
i64 controlDelay = 60;

int currentFigure = -1;
POINT centerCurrentFigure = {-1};

void ResetField()
{
	for (int i = 0; i < gameFieldWidth; i++)
	{
		for (int j = 0; j < gameFieldHeight; j++)
		{
			gameField[i][j] = 0;
		}
	}
}

void CheckOnBurningLine()
{
	bool list[gameFieldHeight] = { false };
	int lineCounter = 0;
	for (int j = 0; j < gameFieldHeight; j++)
	{
		int count = 0;
		for (int i = 0; i < gameFieldWidth; i++)
		{
			if (gameField[i][j] == 1)
			{
				count++;
			}
		}
		if (count == 10)
		{
			list[j] = true;
			lineCounter++;
		}
	}
	
	if (lineCounter > 0)
	{
		// TODO: SUPER WAU EFFECT!!!!
		for (int j = gameFieldHeight - 1; j >= 0; j--)
		{
			if (list[j])
			{
				for (int i = 0; i < gameFieldWidth; i++)
				{
					gameField[i][j] = 0;
				}

				for (int h = j; h < gameFieldHeight - 1; h++)
				{
					for (int i = 0; i < gameFieldWidth; i++)
					{
						gameField[i][h] = gameField[i][h + 1];
						gameField[i][h + 1] = 0;
					}
				}
			}
		}
	}
}

void SpawnFigure()
{
	currentFigure = rand() % 7;

	switch (currentFigure)
	{
		// palka
		case 0:
		{
			gameField[3][19] = 2;
			gameField[4][19] = 2;
			gameField[5][19] = 2;
			gameField[6][19] = 2;
			centerCurrentFigure.x = 5;
			centerCurrentFigure.y = 19;
		} break;
		// cubik
		case 1:
		{
			gameField[4][19] = 2;
			gameField[4][18] = 2;
			gameField[5][19] = 2;
			gameField[5][18] = 2;
		} break;
		// z - obrazn
		case 2:
		{
			gameField[4][19] = 2;
			gameField[5][19] = 2;
			gameField[5][18] = 2;
			gameField[6][18] = 2;
			centerCurrentFigure.x = 5;
			centerCurrentFigure.y = 19;
		} break;
		// z - obrazn flip horizontal
		case 3:
		{
			gameField[4][18] = 2;
			gameField[5][18] = 2;
			gameField[5][19] = 2;
			gameField[6][19] = 2;
			centerCurrentFigure.x = 5;
			centerCurrentFigure.y = 19;
		} break;
		// bukva G
		case 4:
		{
			gameField[4][18] = 2;
			gameField[4][19] = 2;
			gameField[5][19] = 2;
			gameField[6][19] = 2;
			centerCurrentFigure.x = 5;
			centerCurrentFigure.y = 18;
		} break;
		// bukva G flip horizontal
		case 5:
		{
			gameField[6][18] = 2;
			gameField[4][19] = 2;
			gameField[5][19] = 2;
			gameField[6][19] = 2;
			centerCurrentFigure.x = 5;
			centerCurrentFigure.y = 18;
		} break;
		// T obrazn
		case 6:
		{
			gameField[4][19] = 2;
			gameField[5][19] = 2;
			gameField[6][19] = 2;
			gameField[5][18] = 2;
			centerCurrentFigure.x = 5;
			centerCurrentFigure.y = 19;
		} break;
		invalid_default
	}
}

int CheckForPlayerFigure()
{
	for (int j = 0; j < gameFieldHeight; j++)
	{
		for (int i = 0; i < gameFieldWidth; i++)
		{
			if (gameField[i][j] == 2)
			{
				return j;
			}
		}
	}
	return -1;
}

int WhatToDoWithPlayerFigure(int lowerY)
{
	if (lowerY == 0)
	{
		return 1;
	}
	else
	{
		for (int i = 0; i < gameFieldWidth; i++)
		{
			for (int j = 1; j < gameFieldHeight; j++)
			{
				if ((gameField[i][j] == 2) && (gameField[i][j - 1] == 1))
				{
					return 1;
				}
			}
		}
	}

	return 2;
}

// just check higher line
bool CheckForGameEnd()
{
	for (int i = 0; i < gameFieldWidth; i++)
	{
		if (gameField[i][gameFieldHeight - 1] == 1)
			return true;
	}
	return false;
}

void UpdateGameTick()
{
	// 0 - reserv
	// 1 - change to 1
	// 2 - lower the figure
	int whatToDo = 0;

	int lowerY = CheckForPlayerFigure();

	whatToDo = WhatToDoWithPlayerFigure(lowerY);

	if (whatToDo == 2)
	{
		for (int i = 0; i < gameFieldWidth; i++)
		{
			for (int j = 1; j < gameFieldHeight; j++)
			{
				if (gameField[i][j] == 2)
				{
					gameField[i][j - 1] = 2;
					gameField[i][j] = 0;
				}
			}
		}
		centerCurrentFigure.y--;
	}
	else if (whatToDo == 1)
	{
		for (int i = 0; i < gameFieldWidth; i++)
		{
			for (int j = 0; j < gameFieldHeight; j++)
			{
				if (gameField[i][j] == 2)
				{
					gameField[i][j] = 1;
				}
			}
		}
	}
	else
	{
		ASSERT(whatToDo);
	}

	CheckOnBurningLine();

	if (CheckForGameEnd())
	{
		ResetField();
	}

}

bool GetMartixAroundCenter(m3* mat)
{
	if (((centerCurrentFigure.x - 1) < 0) || (centerCurrentFigure.x + 1 > (gameFieldWidth - 1)) 
	|| ((centerCurrentFigure.y - 1) < 0) || ((centerCurrentFigure.y + 1) > (gameFieldHeight - 1)))
	{
		return false;
	}

	mat->_11 = gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 1];
	mat->_12 = gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 1];
	mat->_13 = gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 1];
	mat->_21 = gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 0];
	mat->_22 = gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 0];
	mat->_23 = gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 0];
	mat->_31 = gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 1];
	mat->_32 = gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 1];
	mat->_33 = gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 1];

	return true;
}

void SetMatrixValueAroundCenter(m3* mat)
{
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 1] = mat->_11;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 1] = mat->_12;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 1] = mat->_13;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 0] = mat->_21;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 0] = mat->_22;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 0] = mat->_23;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 1] = mat->_31;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 1] = mat->_32;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 1] = mat->_33;
}

bool rotateLeft(m3* mat)
{
	m3 temp = *mat;
	mat->_13 = temp._11;
	mat->_23 = temp._12;
	mat->_33 = temp._13;
	mat->_32 = temp._23;
	mat->_31 = temp._33;
	mat->_21 = temp._32;
	mat->_11 = temp._31;
	mat->_12 = temp._21;

	for (int i = 0; i < 9; i++)
	{
		if ((mat->data[i] == 2) && (temp.data[i] == 1))
		{
			return false;
		}
	}

	return true;
}

bool rotateRight(m3* mat)
{
	m3 temp = *mat;
	mat->_11 = temp._13;
	mat->_21 = temp._12;
	mat->_31 = temp._11;
	mat->_32 = temp._21;
	mat->_33 = temp._31;
	mat->_23 = temp._32;
	mat->_13 = temp._33;
	mat->_12 = temp._23;

	for (int i = 0; i < 9; i++)
	{
		if ((mat->data[i] == 2) && (temp.data[i] == 1))
		{
			return false;
		}
	}

	return true;
}

bool GetMartixAroundCenter(m4* mat)
{
	if (((centerCurrentFigure.x - 2) < 0) || (centerCurrentFigure.x + 1 > (gameFieldWidth - 1))
		|| ((centerCurrentFigure.y - 1) < 0) || ((centerCurrentFigure.y + 2) > (gameFieldHeight - 1)))
	{
		return false;
	}

	mat->_11 = gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 2];
	mat->_12 = gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 2];
	mat->_13 = gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 2];
	mat->_14 = gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 2];
	mat->_21 = gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 1];
	mat->_22 = gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 1];
	mat->_23 = gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 1];
	mat->_24 = gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 1];
	mat->_31 = gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 0];
	mat->_32 = gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 0];
	mat->_33 = gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 0];
	mat->_34 = gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 0];
	mat->_41 = gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y - 1];
	mat->_42 = gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 1];
	mat->_43 = gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 1];
	mat->_44 = gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 1];

	return true;
}

void SetMatrixValueAroundCenter(m4* mat)
{
	gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 2] = mat->_11;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 2] = mat->_12;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 2] = mat->_13;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 2] = mat->_14;
	gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 1] = mat->_21;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 1] = mat->_22;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 1] = mat->_23;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 1] = mat->_24;
	gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 0] = mat->_31;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 0] = mat->_32;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 0] = mat->_33;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 0] = mat->_34;
	gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y - 1] = mat->_41;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 1] = mat->_42;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 1] = mat->_43;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 1] = mat->_44;
}

bool rotateLeft(m4* mat)
{
	m4 temp = *mat;
	*mat = Transpose(*mat);

	for (int i = 0; i < 16; i++)
	{
		if ((mat->data[i] == 2) && (temp.data[i] == 1))
		{
			return false;
		}
	}

	return true;
}

void TurnPlayerFigure(int direction)
{
	if (currentFigure > 1)
	{
		if (direction == VK_LEFT)
		{
			m3 mat = {};
			if (GetMartixAroundCenter(&mat))
			{
				if (rotateLeft(&mat))
				{
					SetMatrixValueAroundCenter(&mat);
				}
			}
		}
		else if (direction == VK_RIGHT)
		{
			m3 mat = {};
			if (GetMartixAroundCenter(&mat))
			{
				if (rotateRight(&mat))
				{
					SetMatrixValueAroundCenter(&mat);
				}
			}
		}
	}
	else if (currentFigure == 0)
	{
		m4 mat = {};
		if (GetMartixAroundCenter(&mat))
		{
			if (rotateLeft(&mat))
			{
				SetMatrixValueAroundCenter(&mat);
			}
		}
	}
}

void MovePlayerFigure(int direction)
{
	if (direction == VK_LEFT)
	{
		bool allowToMove = true;
		for (int i = 0; i < gameFieldWidth; i++)
		{
			for (int j = 0; j < gameFieldHeight; j++)
			{
				if (gameField[i][j] == 2)
				{
					if ((gameField[i - 1][j] == 1) || (i == 0))
					{
						allowToMove = false;
					}
				}
			}
		}

		if (allowToMove)
		{
			for (int i = 1; i < gameFieldWidth; i++)
			{
				for (int j = 0; j < gameFieldHeight; j++)
				{
					if (gameField[i][j] == 2)
					{
						gameField[i][j] = 0;
						gameField[i - 1][j] = 2;
					}
				}
			}
			centerCurrentFigure.x--;
		}
	}
	else if (direction == VK_RIGHT)
	{
		bool allowToMove = true;
		for (int i = 0; i < gameFieldWidth; i++)
		{
			for (int j = 0; j < gameFieldHeight; j++)
			{
				if (gameField[i][j] == 2)
				{
					if ((gameField[i + 1][j] == 1) || (i == gameFieldWidth - 1))
					{
						allowToMove = false;
					}
				}
			}
		}

		if (allowToMove)
		{
			for (int i = gameFieldWidth - 1; i >= 0; i--)
			{
				for (int j = 0; j < gameFieldHeight; j++)
				{
					if (gameField[i][j] == 2)
					{
						gameField[i][j] = 0;
						gameField[i + 1][j] = 2;
					}
				}
			}
			centerCurrentFigure.x++;
		}
	}
	else if (direction == VK_DOWN)
	{
		int lowerY = CheckForPlayerFigure();
		if (WhatToDoWithPlayerFigure(lowerY) == 2)
		{
			for (int i = 0; i < gameFieldWidth; i++)
			{
				for (int j = 1; j < gameFieldHeight; j++)
				{
					if (gameField[i][j] == 2)
					{
						gameField[i][j - 1] = 2;
						gameField[i][j] = 0;
					}
				}
			}
			centerCurrentFigure.y--;
		}
	}
	else
	{
		ASSERT(0);
	}
}

bool KeyDown(int key)
{
	if (GetAsyncKeyState(key))
		return true;
	else
		return false;
}

bool KeyPressed(int key)
{
	if (GetAsyncKeyState(key) && !keyState[key])
	{
		keyState[key] = true;
		return true;
	}
	return false;
}

bool KeyReleased(int key)
{
	if (!GetAsyncKeyState(key) && keyState[key])
	{
		keyState[key] = false;
		return true;
	}
	return false;
}

void Control()
{
	if (KeyPressed(VK_LEFT))
	{
		controlLeftOnePressElapsed = 0;
		MovePlayerFigure(VK_LEFT);
	}
	if (KeyPressed(VK_RIGHT))
	{
		controlRightOnePressElapsed = 0;
		MovePlayerFigure(VK_RIGHT);
	}
	if (KeyPressed(VK_DOWN))
	{
		controlDownOnePressElapsed = 0;
		MovePlayerFigure(VK_DOWN);
	}
	if (KeyPressed(VK_UP))
	{
		TurnPlayerFigure(VK_LEFT);
	}
	if (KeyPressed(0x58)) // x
	{
		TurnPlayerFigure(VK_RIGHT);
	}
	if (KeyPressed(0x5A)) // z
	{
		TurnPlayerFigure(VK_LEFT);
	}

	if (KeyDown(VK_DOWN))
	{
		controlDownOnePressElapsed += currentFrame - lastFrame;
		if (controlDownOnePressElapsed >= controlOnePressDelay)
		{
			controlDownElapsed += currentFrame - lastFrame;
			if (controlDownElapsed >= controlDelay)
			{
				controlDownElapsed = 0;
				MovePlayerFigure(VK_DOWN);
			}
		}
	}

	if (KeyDown(VK_LEFT))
	{
		controlLeftOnePressElapsed += currentFrame - lastFrame;
		if (controlLeftOnePressElapsed >= controlOnePressDelay)
		{
			controlLeftElapsed += currentFrame - lastFrame;
			if (controlLeftElapsed >= controlDelay)
			{
				controlLeftElapsed = 0;
				MovePlayerFigure(VK_LEFT);
			}
		}
	}

	if (KeyDown(VK_RIGHT))
	{
		controlRightOnePressElapsed += currentFrame - lastFrame;
		if (controlRightOnePressElapsed >= controlOnePressDelay)
		{
			controlRightElapsed += currentFrame - lastFrame;
			if (controlRightElapsed >= controlDelay)
			{
				controlRightElapsed = 0;
				MovePlayerFigure(VK_RIGHT);
			}
		}
	}

	KeyReleased(VK_LEFT);
	KeyReleased(VK_RIGHT);
	KeyReleased(VK_DOWN);
	KeyReleased(VK_UP);
	KeyReleased(0x58);
	KeyReleased(0x5A);
}

void DrawingFillRectangle(Win32__Bitmap_Offscreen_Buffer* buffer, int x_pos, int y_pos, int width, int height, int type)
{
	if (type == 0)
	{
		u8* row = (u8*)buffer->memory;
		row += buffer->pitch * y_pos;
		for (int y = 0; y < height; y++)
		{
			u32* pixel = (u32*)row;
			pixel += x_pos;
			for (int x = 0; x < width; x++)
			{
				u8 red = y * 3;
				u8 green = x * 10;
				u8 blue = 125 + y;
				u8 reserv = 0;

				*pixel++ = (blue | (green << 8) | (red << 16) | (reserv << 24));
			}
			row += buffer->pitch;
		}
	}
	else if (type == 1)
	{
		u8* row = (u8*)buffer->memory;
		row += buffer->pitch * y_pos;
		for (int y = 0; y < height; y++)
		{
			u32* pixel = (u32*)row;
			pixel += x_pos;
			for (int x = 0; x < width; x++)
			{
				u8 red = 128;
				u8 green = 128;
				u8 blue = 128;
				u8 reserv = 0;

				*pixel++ = (blue | (green << 8) | (red << 16) | (reserv << 24));
			}
			row += buffer->pitch;
		}
	}
}

void RenderTetrisGame(Win32__Bitmap_Offscreen_Buffer* buffer)
{
	int posx = buffer->width / 2.5f;
	int posy = 20;
	f32 widthCoef = (f32)buffer->width / 800;
	f32 heightCoef = (f32)buffer->height / 450;
	f32 scaleCoef = 20.0f;

	int border = 2;
	// drawing glass
	DrawingFillRectangle(buffer, posx, posy, gameFieldWidth * scaleCoef * widthCoef, gameFieldHeight * scaleCoef * heightCoef, 1);

	for (int i = 0; i < gameFieldWidth; i++)
	{
		for (int j = 0; j < gameFieldHeight; j++)
		{
			if (gameField[i][j] > 0)
			{
				DrawingFillRectangle(buffer, i * scaleCoef * widthCoef + posx, j * scaleCoef * heightCoef + posy, scaleCoef * widthCoef - border, scaleCoef * heightCoef - border, 0);
			}
		}
	}
}
// End of Tetris realize

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

	// begin of Tetris init
	srand(GetTimeStampMilliSecond());
	// end of Tetris init

	HDC deviceContext = GetDC(hMainWnd);

	int xOffset = 0;
	int yOffset = 0;

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

		// begin of Render and control Tetris Game
		currentFrame = GetTimeStampMilliSecond();
		timeElapsed += currentFrame - lastFrame;
		Control();
		lastFrame = currentFrame;
		if (timeElapsed >= speedMillisecond)
		{
			// call all of this every tick and provoke tick when use the down arrow key
			UpdateGameTick();
			//con::Outf(L"tick\n");
			if (CheckForPlayerFigure() < 0)
			{
				SpawnFigure();
			}
			timeElapsed = 0;
		}
		RenderWeirdGradient(&backbuffer, xOffset, yOffset);
		RenderTetrisGame(&backbuffer);
		// end of Render and control Tetris Game

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