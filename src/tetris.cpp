#include "tetris.h"

void GameOutputSound(Game_Sound_Output_Buffer* soundBuffer, int toneHz)
{
	static f32 tSine;
	i16 toneVolume = 3000;
	int wavePeriod = soundBuffer->samplesPerSecond / toneHz;

	i16* sampleOut = soundBuffer->samples;
	for (int sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; sampleIndex++)
	{
		f32 sineValue = Sin(tSine);
		i16 sampleValue = (i16)(sineValue * toneVolume);
		*sampleOut++ = sampleValue;
		*sampleOut++ = sampleValue;

		tSine += 2.0f * PI_32 * 1.0f / (f32)wavePeriod;
	}
}

void RenderWeirdGradient(Game_Bitmap_Offscreen_Buffer* buffer, int xOffset, int yOffset)
{
	u8* row = (u8*)buffer->memory;
	for (int y = 0; y < buffer->height; y++)
	{
		u32* pixel = (u32*)row;
		for (int x = 0; x < buffer->width; x++)
		{
			u8 red = 0;
			u8 green = (y + yOffset);
			u8 blue = (x + xOffset);
			u8 reserv = 0;

			*pixel++ = (blue | (green << 8) | (red << 16) | (reserv << 24));
		}

		row += buffer->pitch;
	}
}

void GameUpdateAndRender(Game_Memory* memory, Game_Input* input, Game_Bitmap_Offscreen_Buffer* buffer, Game_Sound_Output_Buffer* soundBuffer, i64 currentFrame, MPRecieveInfo* info)
{
	// Learnings
	
	Game_State* gameState = (Game_State*)memory->permanentStorage;
	if (!memory->isInitialized)
	{
		gameState->xOffset = 0;
		gameState->yOffset = 0;
		gameState->toneHz = 256;

		// TODO: This may be more appropriate to do in the platform layer
		memory->isInitialized = true;
	}

	
	//GameOutputSound(soundBuffer, gameState->toneHz);
	RenderWeirdGradient(buffer, gameState->xOffset, gameState->yOffset);

	// Tetris
	if (!gameInit)
	{
		InitTetrisGame(&hostGameState);
		gameInit = true;
	}

	if (multiplayerState == 0) // Single
	{
		UpdateTetrisGame(&hostGameState, currentFrame);
		RenderTetrisGame(buffer, (int)(buffer->width / 2.5f), &hostGameState.field);

		// Fonts
		wchar_t wcharStringBuffer[30] = {};
		_itow_s(hostGameState.currentScore, wcharStringBuffer, 10, 10);
		WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, wcharStringBuffer, 40, 170);
		WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, L"max:", 40, 110);
		_itow_s(hostGameState.maxScore, wcharStringBuffer, 10, 10);
		WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, wcharStringBuffer, 40, 50);
	}
	else if (multiplayerState == 1) // Join multiplayer
	{
		if(!tryToConnectClient) { ProccesIPInput(&ipStructure); }
		
		if (ipStructure.ipLength > 0)
		{
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, ipStructure.ipString, 10, 50);
		}

		if (runMPGame)
		{
			UpdateTetrisGame(&hostGameState, currentFrame);

			RenderTetrisGame(buffer, (int)(buffer->width / 6.5f), &hostGameState.field);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, L"Ты:", 10, 620);

			// fonts
			wchar_t wcharStringBuffer[30] = {};
			_itow_s(hostGameState.currentScore, wcharStringBuffer, 10, 10);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, wcharStringBuffer, 10, 170);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, L"max:", 10, 110);
			_itow_s(hostGameState.maxScore, wcharStringBuffer, 10, 10);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, wcharStringBuffer, 10, 50);

			RenderTetrisGame(buffer, (int)(buffer->width / 1.5f), info->field);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, L"Соперник:", 800, 620);

			// fonts rival
			_itow_s(info->currentScore, wcharStringBuffer, 10, 10);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, wcharStringBuffer, 600, 170);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, L"max:", 600, 110);
			_itow_s(info->maxScore, wcharStringBuffer, 10, 10);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, wcharStringBuffer, 600, 50);
		}

		/*
		1.	Initialize Winsock.
		2.	Create a socket.
		3.	Connect to the server.
		4.	Send and receive data.
		5.	Disconnect.
		*/
	}
	else if (multiplayerState == 2) // Host multiplayer
	{
		if (runMPGame)
		{
			UpdateTetrisGame(&hostGameState, currentFrame);

			RenderTetrisGame(buffer, (int)(buffer->width / 6.5f), &hostGameState.field);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, L"Ты:", 10, 620);

			// fonts
			wchar_t wcharStringBuffer[30] = {};
			_itow_s(hostGameState.currentScore, wcharStringBuffer, 10, 10);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, wcharStringBuffer, 10, 170);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, L"max:", 10, 110);
			_itow_s(hostGameState.maxScore, wcharStringBuffer, 10, 10);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, wcharStringBuffer, 10, 50);

			RenderTetrisGame(buffer, (int)(buffer->width / 1.5f), info->field);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, L"Соперник:", 800, 620);

			// fonts rival
			_itow_s(info->currentScore, wcharStringBuffer, 10, 10);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, wcharStringBuffer, 600, 170);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, L"max:", 600, 110);
			_itow_s(info->maxScore, wcharStringBuffer, 10, 10);
			WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, wcharStringBuffer, 600, 50);
		}
		/*
		1.	Initialize Winsock.
		2.	Create a socket.
		3.	Bind the socket.
		4.	Listen on the socket for a client.
		5.	Accept a connection from a client.
		6.	Receive and send data.
		7.	Disconnect.
		*/
	}
}

// Tetris

void ResetField(Tetris_Game_Field* field)
{
	for (int i = 0; i < field->width; i++)
	{
		for (int j = 0; j < field->height; j++)
		{
			field->data[i][j] = 0;
		}
	}
}

void CheckOnBurningLine(Tetris_Game_Field* field, int* currentScore)
{
	// TODO: variable height
	bool list[20] = { false };
	int lineCounter = 0;
	for (int j = 0; j < field->height; j++)
	{
		int count = 0;
		for (int i = 0; i < field->width; i++)
		{
			if (field->data[i][j] == 1)
			{
				count++;
			}
		}
		if (count == 10)
		{
			list[j] = true;
			lineCounter++;
		}
	}

	if (lineCounter > 0)
	{
		*currentScore += lineCounter * lineCounter * 10;

		// TODO: SUPER WAU EFFECT!!!!
		for (int j = field->height - 1; j >= 0; j--)
		{
			if (list[j])
			{
				for (int i = 0; i < field->width; i++)
				{
					field->data[i][j] = 0;
				}

				for (int h = j; h < field->height - 1; h++)
				{
					for (int i = 0; i < field->width; i++)
					{
						field->data[i][h] = field->data[i][h + 1];
						field->data[i][h + 1] = 0;
					}
				}
			}
		}
	}
}

bool SpawnFigure(Tetris_Game_State* state)
{
	Game_Point* centerCurrentFigure = &state->centerCurrentFigure;
	Tetris_Game_Field* field = &state->field;

	if (state->currentFigure < 0)
	{
		state->currentFigure = rand() % 7;
		state->nextFigure = rand() % 7;
	}
	else
	{
		state->currentFigure = state->nextFigure;
		state->nextFigure = rand() % 7;
	}

	int heightPoint = field->height - 1;
	int widthPoint = field->width / 2;


	switch (state->currentFigure)
	{
		// palka
		case 0:
		{
			// failure check
			if (field->data[widthPoint - 2][heightPoint] == 1) return false;
			if (field->data[widthPoint - 1][heightPoint] == 1) return false;
			if (field->data[widthPoint    ][heightPoint] == 1) return false;
			if (field->data[widthPoint + 1][heightPoint] == 1) return false;

			field->data[widthPoint - 2][heightPoint] = 2;
			field->data[widthPoint - 1][heightPoint] = 2;
			field->data[widthPoint    ][heightPoint] = 2;
			field->data[widthPoint + 1][heightPoint] = 2;
			centerCurrentFigure->x = widthPoint;
			centerCurrentFigure->y = heightPoint;
		} break;
		// cubik
		case 1:
		{
			// failure check
			if (field->data[widthPoint - 1][heightPoint    ] == 1) return false;
			if (field->data[widthPoint - 1][heightPoint - 1] == 1) return false;
			if (field->data[widthPoint    ][heightPoint    ] == 1) return false;
			if (field->data[widthPoint    ][heightPoint - 1] == 1) return false;

			field->data[widthPoint - 1][heightPoint	   ] = 2;
			field->data[widthPoint - 1][heightPoint - 1] = 2;
			field->data[widthPoint	  ][heightPoint    ] = 2;
			field->data[widthPoint    ][heightPoint - 1] = 2;
		} break;
		// z - obrazn
		case 2:
		{
			// failure check
			if (field->data[widthPoint - 1][heightPoint    ] == 1) return false;
			if (field->data[widthPoint    ][heightPoint    ] == 1) return false;
			if (field->data[widthPoint    ][heightPoint - 1] == 1) return false;
			if (field->data[widthPoint + 1][heightPoint - 1] == 1) return false;

			field->data[widthPoint - 1][heightPoint    ] = 2;
			field->data[widthPoint    ][heightPoint    ] = 2;
			field->data[widthPoint    ][heightPoint - 1] = 2;
			field->data[widthPoint + 1][heightPoint - 1] = 2;
			centerCurrentFigure->x = widthPoint;
			centerCurrentFigure->y = heightPoint;
		} break;
		// z - obrazn flip horizontal
		case 3:
		{
			// failure check
			if (field->data[widthPoint - 1][heightPoint - 1] == 1) return false;
			if (field->data[widthPoint    ][heightPoint - 1] == 1) return false;
			if (field->data[widthPoint    ][heightPoint    ] == 1) return false;
			if (field->data[widthPoint + 1][heightPoint    ] == 1) return false;

			field->data[widthPoint - 1][heightPoint - 1] = 2;
			field->data[widthPoint    ][heightPoint - 1] = 2;
			field->data[widthPoint    ][heightPoint    ] = 2;
			field->data[widthPoint + 1][heightPoint    ] = 2;
			centerCurrentFigure->x = widthPoint;
			centerCurrentFigure->y = heightPoint;
		} break;
		// bukva G
		case 4:
		{
			// failure check
			if (field->data[widthPoint - 1][heightPoint - 1] == 1) return false;
			if (field->data[widthPoint - 1][heightPoint    ] == 1) return false;
			if (field->data[widthPoint    ][heightPoint    ] == 1) return false;
			if (field->data[widthPoint + 1][heightPoint    ] == 1) return false;

			field->data[widthPoint - 1][heightPoint - 1] = 2;
			field->data[widthPoint - 1][heightPoint    ] = 2;
			field->data[widthPoint    ][heightPoint    ] = 2;
			field->data[widthPoint + 1][heightPoint    ] = 2;
			centerCurrentFigure->x = widthPoint;
			centerCurrentFigure->y = heightPoint - 1;
		} break;
		// bukva G flip horizontal
		case 5:
		{
			// failure check
			if (field->data[widthPoint + 1][heightPoint - 1] == 1) return false;
			if (field->data[widthPoint - 1][heightPoint    ] == 1) return false;
			if (field->data[widthPoint    ][heightPoint    ] == 1) return false;
			if (field->data[widthPoint + 1][heightPoint    ] == 1) return false;

			field->data[widthPoint + 1][heightPoint - 1] = 2;
			field->data[widthPoint - 1][heightPoint    ] = 2;
			field->data[widthPoint    ][heightPoint    ] = 2;
			field->data[widthPoint + 1][heightPoint    ] = 2;
			centerCurrentFigure->x = widthPoint;
			centerCurrentFigure->y = heightPoint - 1;
		} break;
		// T obrazn
		case 6:
		{
			// failure check
			if (field->data[widthPoint - 1][heightPoint    ] == 1) return false;
			if (field->data[widthPoint    ][heightPoint    ] == 1) return false;
			if (field->data[widthPoint + 1][heightPoint    ] == 1) return false;
			if (field->data[widthPoint    ][heightPoint - 1] == 1) return false;

			field->data[widthPoint - 1][heightPoint    ] = 2;
			field->data[widthPoint    ][heightPoint    ] = 2;
			field->data[widthPoint + 1][heightPoint    ] = 2;
			field->data[widthPoint    ][heightPoint - 1] = 2;
			centerCurrentFigure->x = widthPoint;
			centerCurrentFigure->y = heightPoint;
		} break;
		invalid_default
	}

	return true;
}

int CheckForPlayerFigure(Tetris_Game_Field* field)
{
	for (int j = 0; j < field->height; j++)
	{
		for (int i = 0; i < field->width; i++)
		{
			if (field->data[i][j] == 2)
			{
				return j;
			}
		}
	}
	return -1;
}

int WhatToDoWithPlayerFigure(int lowerY, Tetris_Game_Field* field)
{
	if (lowerY == 0)
	{
		return 1;
	}
	else
	{
		for (int i = 0; i < field->width; i++)
		{
			for (int j = 1; j < field->height; j++)
			{
				if ((field->data[i][j] == 2) && (field->data[i][j - 1] == 1))
				{
					return 1;
				}
			}
		}
	}

	return 2;
}

// just check higher line
//bool CheckForGameEnd(Tetris_Game_Field* field)
//{
//	for (int i = 0; i < field->width; i++)
//	{
//		if (field->data[i][field->height - 1] == 1)
//			return true;
//	}
//	return false;
//}

void UpdateGameTick(Game_Point* centerCurrentFigure, Tetris_Game_Field* field, int* maxScore, int* currentScore)
{
	// 0 - reserv
	// 1 - change to 1
	// 2 - lower the figure
	int whatToDo = 0;

	int lowerY = CheckForPlayerFigure(field);

	whatToDo = WhatToDoWithPlayerFigure(lowerY, field);

	if (whatToDo == 2)
	{
		for (int i = 0; i < field->width; i++)
		{
			for (int j = 1; j < field->height; j++)
			{
				if (field->data[i][j] == 2)
				{
					field->data[i][j - 1] = 2;
					field->data[i][j] = 0;
				}
			}
		}
		centerCurrentFigure->y--;
	}
	else if (whatToDo == 1)
	{
		for (int i = 0; i < field->width; i++)
		{
			for (int j = 0; j < field->height; j++)
			{
				if (field->data[i][j] == 2)
				{
					field->data[i][j] = 1;
				}
			}
		}
	}
	else
	{
		ASSERT(whatToDo);
	}

	CheckOnBurningLine(field, currentScore);

	/*if (CheckForGameEnd(field))
	{
		ResetField(field);
	}*/

}

bool GetMartixAroundCenter(m3* mat, Game_Point* centerCurrentFigure, Tetris_Game_Field* field)
{
	if (((centerCurrentFigure->x - 1) < 0) || (centerCurrentFigure->x + 1 > (field->width - 1))
		|| ((centerCurrentFigure->y - 1) < 0) || ((centerCurrentFigure->y + 1) > (field->height - 1)))
	{
		return false;
	}

	mat->_11 = (f32)field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y + 1];
	mat->_12 = (f32)field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y + 1];
	mat->_13 = (f32)field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y + 1];
	mat->_21 = (f32)field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y - 0];
	mat->_22 = (f32)field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y - 0];
	mat->_23 = (f32)field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y - 0];
	mat->_31 = (f32)field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y - 1];
	mat->_32 = (f32)field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y - 1];
	mat->_33 = (f32)field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y - 1];

	return true;
}

void SetMatrixValueAroundCenter(m3* mat, Game_Point* centerCurrentFigure, Tetris_Game_Field* field)
{
	field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y + 1] = (i32)mat->_11;
	field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y + 1] = (i32)mat->_12;
	field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y + 1] = (i32)mat->_13;
	field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y - 0] = (i32)mat->_21;
	field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y - 0] = (i32)mat->_22;
	field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y - 0] = (i32)mat->_23;
	field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y - 1] = (i32)mat->_31;
	field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y - 1] = (i32)mat->_32;
	field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y - 1] = (i32)mat->_33;
}

bool CheckForRotateFailure(m3* mat, m3* temp)
{
	for (int i = 0; i < 9; i++)
	{
		if ((mat->data[i] == 1) && (temp->data[i] != 1))
		{
			return true;
		}
	}

	return false;
}

bool CheckForRotateFailure(m4* mat, m4* temp)
{
	for (int i = 0; i < 16; i++)
	{
		if ((mat->data[i] == 1) && (temp->data[i] != 1))
		{
			return true;
		}
	}

	return false;
}

bool rotateLeft(m3* mat)
{
	m3 temp = *mat;
	mat->_13 = temp._11;
	mat->_23 = temp._12;
	mat->_33 = temp._13;
	mat->_32 = temp._23;
	mat->_31 = temp._33;
	mat->_21 = temp._32;
	mat->_11 = temp._31;
	mat->_12 = temp._21;

	if (CheckForRotateFailure(mat, &temp))
	{
		return false;
	}

	return true;
}

bool rotateRight(m3* mat)
{
	m3 temp = *mat;
	mat->_11 = temp._13;
	mat->_21 = temp._12;
	mat->_31 = temp._11;
	mat->_32 = temp._21;
	mat->_33 = temp._31;
	mat->_23 = temp._32;
	mat->_13 = temp._33;
	mat->_12 = temp._23;

	if (CheckForRotateFailure(mat, &temp))
	{
		return false;
	}

	return true;
}

bool GetMartixAroundCenter(m4* mat, Game_Point* centerCurrentFigure, Tetris_Game_Field* field)
{
	if (((centerCurrentFigure->x - 2) < 0) || (centerCurrentFigure->x + 1 > (field->width - 1))
		|| ((centerCurrentFigure->y - 1) < 0) || ((centerCurrentFigure->y + 2) > (field->height - 1)))
	{
		return false;
	}

	mat->_11 = (f32)field->data[centerCurrentFigure->x - 2][centerCurrentFigure->y + 2];
	mat->_12 = (f32)field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y + 2];
	mat->_13 = (f32)field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y + 2];
	mat->_14 = (f32)field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y + 2];
	mat->_21 = (f32)field->data[centerCurrentFigure->x - 2][centerCurrentFigure->y + 1];
	mat->_22 = (f32)field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y + 1];
	mat->_23 = (f32)field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y + 1];
	mat->_24 = (f32)field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y + 1];
	mat->_31 = (f32)field->data[centerCurrentFigure->x - 2][centerCurrentFigure->y + 0];
	mat->_32 = (f32)field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y + 0];
	mat->_33 = (f32)field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y + 0];
	mat->_34 = (f32)field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y + 0];
	mat->_41 = (f32)field->data[centerCurrentFigure->x - 2][centerCurrentFigure->y - 1];
	mat->_42 = (f32)field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y - 1];
	mat->_43 = (f32)field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y - 1];
	mat->_44 = (f32)field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y - 1];

	return true;
}

void SetMatrixValueAroundCenter(m4* mat, Game_Point* centerCurrentFigure, Tetris_Game_Field* field)
{
	field->data[centerCurrentFigure->x - 2][centerCurrentFigure->y + 2] = (i32)mat->_11;
	field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y + 2] = (i32)mat->_12;
	field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y + 2] = (i32)mat->_13;
	field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y + 2] = (i32)mat->_14;
	field->data[centerCurrentFigure->x - 2][centerCurrentFigure->y + 1] = (i32)mat->_21;
	field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y + 1] = (i32)mat->_22;
	field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y + 1] = (i32)mat->_23;
	field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y + 1] = (i32)mat->_24;
	field->data[centerCurrentFigure->x - 2][centerCurrentFigure->y + 0] = (i32)mat->_31;
	field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y + 0] = (i32)mat->_32;
	field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y + 0] = (i32)mat->_33;
	field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y + 0] = (i32)mat->_34;
	field->data[centerCurrentFigure->x - 2][centerCurrentFigure->y - 1] = (i32)mat->_41;
	field->data[centerCurrentFigure->x - 1][centerCurrentFigure->y - 1] = (i32)mat->_42;
	field->data[centerCurrentFigure->x - 0][centerCurrentFigure->y - 1] = (i32)mat->_43;
	field->data[centerCurrentFigure->x + 1][centerCurrentFigure->y - 1] = (i32)mat->_44;
}

bool rotateLeft(m4* mat)
{
	m4 temp = *mat;
	*mat = Transpose(*mat);

	if (CheckForRotateFailure(mat, &temp))
	{
		return false;
	}

	return true;
}

void TurnPlayerFigure(int direction, int* currentFigure, Game_Point* centerCurrentFigure, Tetris_Game_Field* field)
{
	if (*currentFigure > 1)
	{
		if (direction == VK_LEFT)
		{
			m3 mat = {};
			if (GetMartixAroundCenter(&mat, centerCurrentFigure, field))
			{
				if (rotateLeft(&mat))
				{
					SetMatrixValueAroundCenter(&mat, centerCurrentFigure, field);
				}
			}
		}
		else if (direction == VK_RIGHT)
		{
			m3 mat = {};
			if (GetMartixAroundCenter(&mat, centerCurrentFigure, field))
			{
				if (rotateRight(&mat))
				{
					SetMatrixValueAroundCenter(&mat, centerCurrentFigure, field);
				}
			}
		}
	}
	else if (*currentFigure == 0)
	{
		m4 mat = {};
		if (GetMartixAroundCenter(&mat, centerCurrentFigure, field))
		{
			if (rotateLeft(&mat))
			{
				SetMatrixValueAroundCenter(&mat, centerCurrentFigure, field);
			}
		}
	}
}

void MovePlayerFigure(int direction, Game_Point* centerCurrentFigure, Tetris_Game_Field* field)
{
	if (direction == VK_LEFT)
	{
		bool allowToMove = true;
		for (int i = 0; i < field->width; i++)
		{
			for (int j = 0; j < field->height; j++)
			{
				if (field->data[i][j] == 2)
				{
					if ((i == 0) || (field->data[i - 1][j] == 1))
					{
						allowToMove = false;
					}
				}
			}
		}

		if (allowToMove)
		{
			for (int i = 1; i < field->width; i++)
			{
				for (int j = 0; j < field->height; j++)
				{
					if (field->data[i][j] == 2)
					{
						field->data[i][j] = 0;
						field->data[i - 1][j] = 2;
					}
				}
			}
			centerCurrentFigure->x--;
		}
	}
	else if (direction == VK_RIGHT)
	{
		bool allowToMove = true;
		for (int i = 0; i < field->width; i++)
		{
			for (int j = 0; j < field->height; j++)
			{
				if (field->data[i][j] == 2)
				{
					if ((i == field->width - 1) || (field->data[i + 1][j] == 1))
					{
						allowToMove = false;
					}
				}
			}
		}

		if (allowToMove)
		{
			for (int i = field->width - 1; i >= 0; i--)
			{
				for (int j = 0; j < field->height; j++)
				{
					if (field->data[i][j] == 2)
					{
						field->data[i][j] = 0;
						field->data[i + 1][j] = 2;
					}
				}
			}
			centerCurrentFigure->x++;
		}
	}
	else if (direction == VK_DOWN)
	{
		int lowerY = CheckForPlayerFigure(field);
		if (WhatToDoWithPlayerFigure(lowerY, field) == 2)
		{
			for (int i = 0; i < field->width; i++)
			{
				for (int j = 1; j < field->height; j++)
				{
					if (field->data[i][j] == 2)
					{
						field->data[i][j - 1] = 2;
						field->data[i][j] = 0;
					}
				}
			}
			centerCurrentFigure->y--;
		}
	}
	else
	{
		ASSERT(0);
	}
}

bool KeyDown(int key)
{
	if (GetAsyncKeyState(key))
		return true;
	else
		return false;
}

bool KeyPressed(int key)
{
	if (GetAsyncKeyState(key) && !keyState[key])
	{
		keyState[key] = true;
		return true;
	}
	return false;
}

bool KeyReleased(int key)
{
	if (!GetAsyncKeyState(key) && keyState[key])
	{
		keyState[key] = false;
		return true;
	}
	return false;
}

void Control(Tetris_Game_State* state)
{
	if (KeyPressed(VK_LEFT))
	{
		controlLeftOnePressElapsed = 0;
		MovePlayerFigure(VK_LEFT, &state->centerCurrentFigure, &state->field);
	}
	if (KeyPressed(VK_RIGHT))
	{
		controlRightOnePressElapsed = 0;
		MovePlayerFigure(VK_RIGHT, &state->centerCurrentFigure, &state->field);
	}
	if (KeyPressed(VK_UP))
	{
		TurnPlayerFigure(VK_LEFT, &state->currentFigure, &state->centerCurrentFigure, &state->field);
	}
	if (KeyPressed(0x58)) // x
	{
		TurnPlayerFigure(VK_RIGHT, &state->currentFigure, &state->centerCurrentFigure, &state->field);
	}
	if (KeyPressed(0x5A)) // z
	{
		TurnPlayerFigure(VK_LEFT, &state->currentFigure, &state->centerCurrentFigure, &state->field);
	}

	if (KeyDown(VK_DOWN))
	{
		controlDownElapsed += state->currentFrame - state->lastFrame;
		if (controlDownElapsed >= controlDelay)
		{
			controlDownElapsed = 0;
			MovePlayerFigure(VK_DOWN, &state->centerCurrentFigure, &state->field);
		}
	}

	if (KeyDown(VK_LEFT))
	{
		controlLeftOnePressElapsed += state->currentFrame - state->lastFrame;
		if (controlLeftOnePressElapsed >= controlOnePressDelay)
		{
			controlLeftElapsed += state->currentFrame - state->lastFrame;
			if (controlLeftElapsed >= controlDelay)
			{
				controlLeftElapsed = 0;
				MovePlayerFigure(VK_LEFT, &state->centerCurrentFigure, &state->field);
			}
		}
	}

	if (KeyDown(VK_RIGHT))
	{
		controlRightOnePressElapsed += state->currentFrame - state->lastFrame;
		if (controlRightOnePressElapsed >= controlOnePressDelay)
		{
			controlRightElapsed += state->currentFrame - state->lastFrame;
			if (controlRightElapsed >= controlDelay)
			{
				controlRightElapsed = 0;
				MovePlayerFigure(VK_RIGHT, &state->centerCurrentFigure, &state->field);
			}
		}
	}

	KeyReleased(VK_LEFT);
	KeyReleased(VK_RIGHT);
	KeyReleased(VK_UP);
	KeyReleased(0x58);
	KeyReleased(0x5A);
}

void UpdateTetrisGame(Tetris_Game_State* gameState, i64 currentFrame)
{
	gameState->currentFrame = currentFrame;

	gameState->timeElapsed += gameState->currentFrame - gameState->lastFrame;
	Control(gameState);
	gameState->lastFrame = gameState->currentFrame;
	if (gameState->timeElapsed >= gameState->speedMillisecond)
	{
		UpdateGameTick(&gameState->centerCurrentFigure, &gameState->field, &gameState->maxScore, &gameState->currentScore);
		if (CheckForPlayerFigure(&gameState->field) < 0)
		{
			if (SpawnFigure(gameState))
			{
				// Allright game be continue
			}
			else
			{
				// TODO: Animated Game Over
				// Game over
				gameState->maxScore = gameState->currentScore;
				ResetField(&gameState->field);
			}
		}
		gameState->timeElapsed = 0;
	}
}

void DrawingFillRectangle(Game_Bitmap_Offscreen_Buffer* buffer, int x_pos, int y_pos, int width, int height, int type)
{
	if (type == 0)
	{
		u8* row = (u8*)buffer->memory;
		row += buffer->pitch * y_pos;
		for (int y = 0; y < height; y++)
		{
			u32* pixel = (u32*)row;
			pixel += x_pos;
			for (int x = 0; x < width; x++)
			{
				u8 red = y + 3;
				u8 green = x + 10;
				u8 blue = 125 + x;
				//u8 reserv = 0;

				*pixel++ = (blue | (green << 8) | (red << 16));
			}
			row += buffer->pitch;
		}
	}
	else if (type == 1)
	{
		u8* row = (u8*)buffer->memory;
		row += buffer->pitch * y_pos;
		for (int y = 0; y < height; y++)
		{
			u32* pixel = (u32*)row;
			pixel += x_pos;
			for (int x = 0; x < width; x++)
			{
				u8 red = 128;
				u8 green = 128;
				u8 blue = 128;
				//u8 reserv = 0;
				f32 opacity = 1.0f;

				u8 prevRed = ((u8*)pixel)[2];
				u8 prevGreen = ((u8*)pixel)[1];
				u8 prevBlue = ((u8*)pixel)[0];

				red = (prevRed - ((prevRed - red) * opacity));
				green = (prevGreen - ((prevGreen - green) * opacity));
				blue = (prevBlue - ((prevBlue - blue) * opacity));

				*pixel++ = (blue | (green << 8) | (red << 16));
			}
			row += buffer->pitch;
		}
	}
}

void RenderTetrisGame(Game_Bitmap_Offscreen_Buffer* buffer, int posx, Tetris_Game_Field* field)
{
	// TODO: colorize figure

	int Tetris_Game_FieldWidth = field->width;
	int Tetris_Game_FieldHeight = field->height;
	int** Tetris_Game_Field = field->data;

	int posy = 20;
	//f32 widthCoef = (f32)buffer->width / 800;
	f32 heightCoef = (f32)buffer->height / 450;
	f32 scaleCoef = 20.0f;

	int border = 2;
	// drawing glass
	DrawingFillRectangle(buffer, posx, posy, (i32)(Tetris_Game_FieldWidth * scaleCoef * heightCoef), (i32)(Tetris_Game_FieldHeight * scaleCoef * heightCoef), 1);

	for (int i = 0; i < Tetris_Game_FieldWidth; i++)
	{
		for (int j = 0; j < Tetris_Game_FieldHeight; j++)
		{
			if (Tetris_Game_Field[i][j] > 0)
			{
				DrawingFillRectangle(buffer, (i32)(i * scaleCoef * heightCoef + posx), (i32)(j * scaleCoef * heightCoef) + posy, (i32)(scaleCoef * heightCoef) - border, (i32)(scaleCoef * heightCoef) - border, 0);
			}
		}
	}
}

void ProccessDigitInput(int key, IPStrucrute* ip)
{
	if (ip->ipLength < 31)
	{
		ip->ipString[ip->ipLength++] = key;
	}
}

void ProccesIPInput(IPStrucrute* ip)
{
	if (KeyPressed(0x30))
	{
		ProccessDigitInput(0x30, ip);
	}
	if (KeyPressed(0x31))
	{
		ProccessDigitInput(0x31, ip);
	}
	if (KeyPressed(0x32))
	{
		ProccessDigitInput(0x32, ip);
	}
	if (KeyPressed(0x33))
	{
		ProccessDigitInput(0x33, ip);
	}
	if (KeyPressed(0x34))
	{
		ProccessDigitInput(0x34, ip);
	}
	if (KeyPressed(0x35))
	{
		ProccessDigitInput(0x35, ip);
	}
	if (KeyPressed(0x36))
	{
		ProccessDigitInput(0x36, ip);
	}
	if (KeyPressed(0x37))
	{
		ProccessDigitInput(0x37, ip);
	}
	if (KeyPressed(0x38))
	{
		ProccessDigitInput(0x38, ip);
	}
	if (KeyPressed(0x39))
	{
		ProccessDigitInput(0x39, ip);
	}
	if (KeyPressed(VK_OEM_PERIOD))
	{
		ProccessDigitInput(46, ip);
	}
	if (KeyPressed(VK_OEM_1))
	{
		ProccessDigitInput(58, ip);
	}
	if (KeyPressed(VK_BACK))
	{
		if (ip->ipLength > 0)
		{
			ip->ipString[ip->ipLength-- - 1] = 0;
		}
	}

	if (KeyPressed(VK_RETURN) && (ip->ipLength > 9))
	{
		// Join
		tryToConnectClient = true;
	}

	KeyReleased(0x30);
	KeyReleased(0x31);
	KeyReleased(0x32);
	KeyReleased(0x33);
	KeyReleased(0x34);
	KeyReleased(0x35);
	KeyReleased(0x36);
	KeyReleased(0x37);
	KeyReleased(0x38);
	KeyReleased(0x39);
	KeyReleased(VK_OEM_PERIOD);
	KeyReleased(VK_OEM_1);
	KeyReleased(VK_BACK);
	KeyReleased(VK_RETURN);
}

void InitTetrisGame(Tetris_Game_State* gameState)
{
	gameState->field.width = 10;
	gameState->field.height = 20;
	gameState->field.data = (int**)malloc(gameState->field.width * sizeof(int*));
	for (int i = 0; i < gameState->field.width; i++)
	{
		gameState->field.data[i] = (int*)malloc(gameState->field.height * sizeof(int));
		for (int j = 0; j < gameState->field.height; j++)
		{
			gameState->field.data[i][j] = 0;
		}
	}

	gameState->lastFrame = 0;
	gameState->currentFrame = 0;
	gameState->timeElapsed = 0;
	gameState->speedMillisecond = 400;

	// Must be -1
	gameState->currentFigure = -1;
	gameState->nextFigure = -1;
	gameState->centerCurrentFigure = { -1 };

	gameState->maxScore = 0;
	gameState->currentScore = 0;
}

void InitGameField(Tetris_Game_Field* field)
{
	field->width = 10;
	field->height = 20;
	field->data = (int**)malloc(field->width * sizeof(int*));
	for (int i = 0; i < field->width; i++)
	{
		field->data[i] = (int*)malloc(field->height * sizeof(int));
	}
}

void ClearInput(IPStrucrute* ipStruct)
{
	ipStruct->ipLength = 0;
	memset(ipStruct->ipString, 0, 32 * sizeof(wchar_t));
}