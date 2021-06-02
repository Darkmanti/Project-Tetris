#include "multiplayer.h"

void GameInfoToBuffer(MPSendInfo* info, char* buffer)
{
	int* ptr = (int*)buffer;

	*ptr++ = *info->currentScore;
	*ptr++ = *info->maxScore;

	*ptr++ = info->field->width;
	*ptr++ = info->field->height;

	for (int i = 0; i < info->field->width; i++)
	{
		for (int j = 0; j < info->field->height; j++)
		{
			*ptr++ = info->field->data[i][j];
		}
	}
}


void BufferToGameInfo(MPRecieveInfo* info, char* buffer)
{
	int* ptr = (int*)buffer;

	info->currentScore = *ptr++;
	info->maxScore = *ptr++;

	info->field->width = *ptr++;
	info->field->height = *ptr++;

	for (int i = 0; i < info->field->width; i++)
	{
		for (int j = 0; j < info->field->height; j++)
		{
			info->field->data[i][j] = *ptr++;
		}
	}
}