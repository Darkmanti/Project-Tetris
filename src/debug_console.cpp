#include "debug_console.h"

namespace con
{
	void InitConsole()
	{
		if (!hConsole)
		{
			AllocConsole();
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTitleW(L"Tetris Console");
		}
		else
		{
			FreeConsole();
		}
	}

	void SetConColor(int color)
	{
		SetConsoleTextAttribute(hConsole, color);
	}

	// push to warnings stack
	#pragma warning(push)
	// disable warning _s functions
	#pragma warning(disable:4996)
	// if wchar_t buffer bigger then unsigned long (u32) ~(4,294,967,295) the output will be erroneous
	#pragma warning(disable:4267)

	void Out(const wchar_t* string)
	{
		WRITE_CONSOLE(hConsole, string, wcslen(string));
	}
	
	void Out(const wchar_t ch)
	{
		WRITE_CONSOLE(hConsole, &ch, 1);
	}

	void Out(int value)
	{
		wchar_t string[12];
		_itow(value, string, 10);
		WRITE_CONSOLE(hConsole, string, wcslen(string));
	}

	void Out(unsigned int value)
	{
		wchar_t string[12];
		_itow(value, string, 10);
		WRITE_CONSOLE(hConsole, string, wcslen(string));
	}
	
	void Out(short value)
	{
		wchar_t string[7];
		_itow(value, string, 10);
		WRITE_CONSOLE(hConsole, string, wcslen(string));
	}
	
	void Out(unsigned short value)
	{
		wchar_t string[7];
		_itow(value, string, 10);
		WRITE_CONSOLE(hConsole, string, wcslen(string));
	}
	
	void Out(long long value)
	{
		wchar_t string[22];
		_i64tow(value, string, 10);
		WRITE_CONSOLE(hConsole, string, wcslen(string));
	}
	
	void Out(unsigned long long value)
	{
		wchar_t string[22];
		_ui64tow(value, string, 10);
		WRITE_CONSOLE(hConsole, string, wcslen(string));
	}
	
	void Out(float value)
	{
		wchar_t string[_CVTBUFSIZE];
		u32 length = U32FloatToWCHAR(value, string);
		WRITE_CONSOLE(hConsole, string, length);
	}
	
	void Out(double value)
	{
		wchar_t string[_CVTBUFSIZE];
		u32 length = U32DoubleToWCHAR(value, string);
		WRITE_CONSOLE(hConsole, string, length);
	}
	
	void Outf(const wchar_t* string, ...)
	{
		wchar_t result[1024];
		wchar_t temp[256];
		memset(result, NULL, 1024 * sizeof(wchar_t));
		memset(temp, NULL, 256 * sizeof(wchar_t));
	
		va_list args;
		va_start(args, string);
	
		int i = 0;
		int length = wcslen(string);
		while (i < length)
		{
			if (string[i] == '%')
			{
				switch (string[i + 1])
				{
					case 's':
					{
						wcscpy(temp, va_arg(args, wchar_t*));
						wcscat(result, temp);
					} break;
					case 'i':
					{
						_itow(va_arg(args, int), temp, 10);
						wcscat(result, temp);
					} break;
					case 'f':
					{
						double value = va_arg(args, double);
						wchar_t string[_CVTBUFSIZE];
						VoidFloatToWCHAR(value, string);
						wcscat(result, string);
					} break;
					case 'm':
					{
						switch (string[i + 2])
						{
							case '2':
							{
								m2 value = va_arg(args, m2);
								value = Transpose(value);
								for (int i = 0; i < 4; i++)
								{
									wchar_t string[_CVTBUFSIZE + 1];
									VoidFloatToWCHAR(value.data[i], string);
	
									if (i == 1)
										wcscat(string, L"\n");
									else
										wcscat(string, L" ");
	
									wcscat(result, string);
								}
							} break;
							case '3':
							{
								m3 value = va_arg(args, m3);
								value = Transpose(value);
								for (int i = 0; i < 9; i++)
								{
									wchar_t string[_CVTBUFSIZE + 1];
									VoidFloatToWCHAR(value.data[i], string);
	
									if (i == 2 || i == 5)
										wcscat(string, L"\n");
									else
										wcscat(string, L" ");
	
									wcscat(result, string);
								}
							} break;
							case '4':
							{
								m4 value = va_arg(args, m4);
								value = Transpose(value);
								for (int i = 0; i < 16; i++)
								{
									wchar_t string[_CVTBUFSIZE + 1];
									VoidFloatToWCHAR(value.data[i], string);
	
									if (i == 3 || i == 7 || i == 11)
										wcscat(string, L"\n");
									else
										wcscat(string, L" ");
	
									wcscat(result, string);
								}
							} break;
							// invalid default
							default:{}
						}
						i = i + 1;
					} break;
					case 'v':
					{
						switch (string[i + 2])
						{
							case '2':
							{
								v2 value = va_arg(args, v2);
								for (int i = 0; i < 2; i++)
								{
									wchar_t string[_CVTBUFSIZE + 1];
									VoidFloatToWCHAR(value.data[i], string);
									if (i != 1)
										wcscat(string, L" ");
									wcscat(result, string);
								}
							} break;
							case '3':
							{
								v3 value = va_arg(args, v3);
								for (int i = 0; i < 3; i++)
								{
									wchar_t string[_CVTBUFSIZE + 1];
									VoidFloatToWCHAR(value.data[i], string);
									if (i != 2)
										wcscat(string, L" ");
									wcscat(result, string);
								}
							} break;
							case '4':
							{
								v4 value = va_arg(args, v4);
								for (int i = 0; i < 4; i++)
								{
									wchar_t string[_CVTBUFSIZE + 1];
									VoidFloatToWCHAR(value.data[i], string);
									if (i != 3)
										wcscat(string, L" ");
									wcscat(result, string);
								}
							} break;
						}
						i = i + 1;
					} break;
				invalid_default()
				}
				i = i + 2;
			}
			else
			{
				result[wcslen(result)] = string[i];
				i++;
			}
		}
		WRITE_CONSOLE(hConsole, result, wcslen(result));
		va_end(args);
	}

	inline void LogAssert(const char* _file, const char* _func, u32 line, const char* _expr, const wchar_t* fmt, ...)
	{
		wchar_t file[256] = L"";
		wchar_t func[256] = L"";
		wchar_t expr[256] = L"";
		mbstowcs(file, _file, 256);
		mbstowcs(func, _func, 256);
		mbstowcs(expr, _expr, 256);
		SetConColor(FOREGROUND_RED);
		Outf(L"[Assertion failed] ");
		SetConColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		Outf(L"Expression (%s) result is false\n", expr);
		Outf(L"File: \"%s\"\nfunction: %s, line: %i.\n", file, func, line);

		// TODO: output parameters, now this is just a garbage
		va_list args;
		va_start(args, fmt);
		Outf(fmt, args);
		va_end(args);
	}

	inline void LogAssert(const char* _file, const char* _func, u32 line, const char* _expr)
	{
		wchar_t file[256] = L"";
		wchar_t func[256] = L"";
		wchar_t expr[256] = L"";
		mbstowcs(file, _file, 256);
		mbstowcs(func, _func, 256);
		mbstowcs(expr, _expr, 256);
		SetConColor(FOREGROUND_RED);
		Outf(L"[Assertion failed] ");
		SetConColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		Outf(L"Expression (%s) result is false\n", expr);
		Outf(L"File: \"%s\"\nfunction: %s, line: %i.\n", file, func, line);
	}
}

// pop of warnings stack
#pragma warning(pop)