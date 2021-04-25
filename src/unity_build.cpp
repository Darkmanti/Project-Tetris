// define some useful macros
#define	internal static

// define debug definitions
#define ASSERTION_ENABLED
#define ASSERTION_SLOW_ENABLED

// define the Unicode char-set
#ifndef UNICODE
#define UNICODE
#endif 
#ifndef _UNICODE
#define _UNICODE
#endif

// main file with program entry point
#include "main.cpp"

// platform-independent layer
#include "tetris.cpp"

// TODO: make it detachable
// debug console for debugging
#include "debug_console.cpp"