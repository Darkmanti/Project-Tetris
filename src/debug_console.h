#pragma once
#include <windows.h>
#include "math.h"
#include "type_converter.h"

// at the moment console be specified for windows, maybe solve it later

#ifdef WIN32
#define WRITE_CONSOLE(handle, str, length) WriteConsoleW(handle, str, length, NULL, NULL)
#endif

#ifdef linux
#define 
#endif

namespace con
{
	internal HANDLE hConsole = NULL;

	void InitConsole();

	void SetConColor(int color);

	void Out(const wchar_t* string);
	void Out(const wchar_t ch);
	void Out(int value);
	void Out(unsigned int value);
	void Out(short value);
	void Out(unsigned short value);
	void Out(long long value);
	void Out(unsigned long long value);
	void Out(float value);
	void Out(double value);

	void Outf(const wchar_t* string, ...);
}