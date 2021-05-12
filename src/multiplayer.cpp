#include "multiplayer.h"

void GameFieldToBuffer(GameField* field, char* buffer)
{
	int* ptr = (int*)buffer;
	*ptr++ = field->width;
	*ptr++ = field->height;

	for (int i = 0; i < field->width; i++)
	{
		for (int j = 0; j < field->height; j++)
		{
			*ptr++ = field->data[i][j];
		}
	}
}


void BufferToGameField(GameField* field, char* buffer)
{
	int* ptr = (int*)buffer;
	field->width = *ptr++;
	field->height = *ptr++;

	for (int i = 0; i < field->width; i++)
	{
		for (int j = 0; j < field->height; j++)
		{
			field->data[i][j] = *ptr++;
		}
	}
}