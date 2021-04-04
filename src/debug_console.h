#pragma once
#include <windows.h>
#include "math.h"
#include "type_converter.h"

// at the moment console be specified for windows, maybe solve it later

#ifdef WIN32
#define WRITE_CONSOLE(handle, str, length) WriteConsoleW(handle, str, length, NULL, NULL)
#define DebugBreakpoint() __debugbreak()
#endif

// TODO: make message box assertion
// TODO: move assertion to another file
#ifdef ASSERTION_ENABLED
#define ASSERT(expr, ...) \
		if(expr) {} \
		else \
		{ \
			con::LogAssert(__FILE__, __func__, __LINE__, #expr, __VA_ARGS__);\
			DebugBreakpoint(); \
		}
#else
#define ASSERT(expr) // doing nothing
#endif

#ifdef ASSERTION_SLOW_ENABLED
#define ASSERT_SLOW ASSERT
#else
#define ASSERT_SLOW(expr) // doing nothing
#endif

#define invalid_default() default:{ASSERT(false);}

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

	inline void LogAssert(const char* file, const char* func, u32 line, const char* expr, const wchar_t* fmt, ...);
	inline void LogAssert(const char* file, const char* func, u32 line, const char* expr);
}