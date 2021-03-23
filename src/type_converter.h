#pragma once
#include <Windows.h>
#include "math.h"

// TODO: Use something normal instead of this craziness
// this applies to all floats converter


#pragma warning(push)
// disable warning _s functions
#pragma warning(disable:4996)
// if wchar_t buffer bigger then unsigned long (u32) ~(4,294,967,295) the output will be erroneous
#pragma warning(disable:4267)

u32 U32FloatToWCHAR(float value, wchar_t* dstStr)
{
	char _string[_CVTBUFSIZE];
	_gcvt(value, 7, _string);
	mbstowcs(dstStr, _string, _CVTBUFSIZE);

	return wcslen(dstStr);
}

u32 U32DoubleToWCHAR(double value, wchar_t* dstStr)
{
	char _string[_CVTBUFSIZE];
	_gcvt(value, 15, _string);
	mbstowcs(dstStr, _string, _CVTBUFSIZE);

	return wcslen(dstStr);
}

void VoidFloatToWCHAR(float value, wchar_t* dstStr)
{
	char _string[_CVTBUFSIZE];
	_gcvt(value, 7, _string);
	mbstowcs(dstStr, _string, _CVTBUFSIZE);
}

void VoidDoubleToWCHAR(double value, wchar_t* dstStr)
{
	char _string[_CVTBUFSIZE];
	_gcvt(value, 15, _string);
	mbstowcs(dstStr, _string, _CVTBUFSIZE);
}

#pragma warning(pop)