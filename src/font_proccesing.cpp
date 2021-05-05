#include "font_proccesing.h"

void InitFont(Font* font, f32 heightFont, i32 glyphs, const wchar_t* fileName, i32 firstChar, i32 width, i32 height)
{
	// TODO: platform independent ReadFile
	DWORD read;
	HANDLE file = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	LARGE_INTEGER fileSize;
	GetFileSizeEx(file, &fileSize);
	u8* data = (u8*)malloc(fileSize.QuadPart);
	memset(data, 0, fileSize.QuadPart);
	ReadFile(file, data, fileSize.QuadPart, &read, NULL);
	CloseHandle(file);

	// TODO: check for errors
	font->bakedFontBitmap = (u8*)malloc(width * height);
	font->cdata = (stbtt_bakedchar*)malloc(glyphs * sizeof(stbtt_bakedchar));

	stbtt_BakeFontBitmap(data, 0, heightFont, font->bakedFontBitmap, width, height, firstChar, glyphs, font->cdata);
	// can free ttf_buffer at this point
	// can free temp_bitmap at this point

	font->firstChar = firstChar;
	font->heightBitMap = height;
	font->widthBitMap = width;
	font->glyphs = glyphs;

	// write bitmap on disk
	stbi_write_bmp("bitmap.bmp", width, height, 1, font->bakedFontBitmap);

	free(data);
}

void WriteLetter(Game_Bitmap_Offscreen_Buffer* buffer, v4* color, stbtt_aligned_quad* q, Font* font)
{
	// Baked bitmap points
	int bitmapLTX = (i32)(q->s0 * font->widthBitMap);
	int bitmapLTY = (i32)(q->t0 * font->heightBitMap);
	int bitmapRBX = (i32)(q->s1 * font->widthBitMap);
	int bitmapRBY = (i32)(q->t1 * font->heightBitMap);

	// Buffer bitmap points
	int bufferLTX = (i32)(q->x0);
	int bufferLTY = (i32)(q->y0);
	int bufferRBX = (i32)(q->x1);
	int bufferRBY = (i32)(q->y1);

	// Baked Bitmap pointer
	u8* bBitmap = font->bakedFontBitmap;
	i32 bBitmapPitch = 1 * font->widthBitMap;
	bBitmap += bBitmapPitch * bitmapLTY;

	// buffer pointer
	u8* row = (u8*)buffer->memory;
	row += buffer->pitch * Abs(bufferLTY);

	// x counter
	int xLength = bufferRBX - bufferLTX;
	// y counter
	int yLength = bufferRBY - bufferLTY;

	// TODO: add more checks for boundaries
	if (yLength > Abs(bufferLTY))
	{
		yLength = yLength - bufferRBY + 1;
	}

	for (int i = 0; i < yLength; i++)
	{
		u32* pixel = (u32*)row;
		pixel += bufferLTX;

		u8* bBitmapPixel = bBitmap;
		bBitmapPixel += bitmapLTX;

		for (int j = 0; j < xLength; j++)
		{
			if (*bBitmapPixel++)
			{
				u8 red =	(u8)color->x;
				u8 green =	(u8)color->y;
				u8 blue =	(u8)color->z;
				u8 reserv = (u8)color->w;

				*pixel = (blue | (green << 8) | (red << 16) | (reserv << 24));
			}
			pixel++;
		}

		row -= buffer->pitch;
		bBitmap += bBitmapPitch;
	}
}

void WriteFont(Game_Bitmap_Offscreen_Buffer* buffer, v4 color, Font* font, const wchar_t* string, int x, int y)
{
	y = -y;
	f32 _x = (f32)x;
	f32 _y = (f32)y;

	// TODO: Batching this letters
	while (*string)
	{
		if (*string >= font->firstChar && *string < font->glyphs)
		{
			stbtt_aligned_quad q = {};
			stbtt_GetBakedQuad(font->cdata, font->widthBitMap, font->heightBitMap, *string - font->firstChar, &_x, &_y, &q, 1);
			WriteLetter(buffer, &color, &q, font);
		}
		string++;
	}
}

void ClearFont(Font* font)
{
	font->firstChar = 0;
	font->widthBitMap = 0;
	font->heightBitMap = 0;
	font->firstChar = 0;
	font->glyphs = 0;

	if (font->cdata != NULL)
	{
		free(font->cdata);
		font->cdata = NULL;
	}
	if (font->bakedFontBitmap != NULL)
	{
		free(font->bakedFontBitmap);
		font->bakedFontBitmap = NULL;
	}
}