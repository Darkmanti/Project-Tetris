// define some usefull macros
#define	internal static

// define the unicode char-set
#ifndef UNICODE
#define UNICODE
#endif 
#ifndef _UNICODE
#define _UNICODE
#endif

// main file with program entry point
#include "main.cpp"

// platform layer fo windows
#include "win_platform.cpp"

// TODO: maybe make it detachable
// dbug console for debuging
#include "debug_console.cpp"