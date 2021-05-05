#pragma once
#include "stb\stb_truetype.h"
#include "math.h"

// for debuging baked bitmap
#include "stb\stb_image_write.h"

struct Font
{
	stbtt_bakedchar* cdata;
	i32 widthBitMap;
	i32 heightBitMap;
	i32 firstChar;
	i32 glyphs;
	u8* bakedFontBitmap;
};

void InitFont(Font* font, f32 heightFont, i32 glyphs, const wchar_t* fileName, i32 firstChar, i32 width, i32 height);

// NOTE: must use string with one line
// TODO: make line carry
// TODO: implement this in main filling loop
void WriteFont(Game_Bitmap_Offscreen_Buffer* buffer, v4 color, Font* font, const wchar_t* string, int x, int y);

void ClearFont(Font* font);