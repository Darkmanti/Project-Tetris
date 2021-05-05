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

void GameUpdateAndRender(Game_Input* input, Game_Bitmap_Offscreen_Buffer* buffer, Game_Sound_Output_Buffer* soundBuffer)
{
	// Learnings
	static int xOffset = 0;
	static int yOffset = 0;
	static int toneHz = 256;

	GameOutputSound(soundBuffer, toneHz);
	RenderWeirdGradient(buffer, xOffset, yOffset);

	// Tetris
	UpdateTetrisGame();
	RenderTetrisGame(buffer);

	// Fonts
	WriteFont(buffer, V4(247.0f, 70.0f, 51.0f, 0.0f), &font, L"Сообщение", 10, 50);
}

// Tetris

void ResetField()
{
	for (int i = 0; i < gameFieldWidth; i++)
	{
		for (int j = 0; j < gameFieldHeight; j++)
		{
			gameField[i][j] = 0;
		}
	}
}

void CheckOnBurningLine()
{
	bool list[gameFieldHeight] = { false };
	int lineCounter = 0;
	for (int j = 0; j < gameFieldHeight; j++)
	{
		int count = 0;
		for (int i = 0; i < gameFieldWidth; i++)
		{
			if (gameField[i][j] == 1)
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
		// TODO: SUPER WAU EFFECT!!!!
		for (int j = gameFieldHeight - 1; j >= 0; j--)
		{
			if (list[j])
			{
				for (int i = 0; i < gameFieldWidth; i++)
				{
					gameField[i][j] = 0;
				}

				for (int h = j; h < gameFieldHeight - 1; h++)
				{
					for (int i = 0; i < gameFieldWidth; i++)
					{
						gameField[i][h] = gameField[i][h + 1];
						gameField[i][h + 1] = 0;
					}
				}
			}
		}
	}
}

void SpawnFigure()
{
	currentFigure = rand() % 7;

	switch (currentFigure)
	{
		// palka
	case 0:
	{
		gameField[3][19] = 2;
		gameField[4][19] = 2;
		gameField[5][19] = 2;
		gameField[6][19] = 2;
		centerCurrentFigure.x = 5;
		centerCurrentFigure.y = 19;
	} break;
	// cubik
	case 1:
	{
		gameField[4][19] = 2;
		gameField[4][18] = 2;
		gameField[5][19] = 2;
		gameField[5][18] = 2;
	} break;
	// z - obrazn
	case 2:
	{
		gameField[4][19] = 2;
		gameField[5][19] = 2;
		gameField[5][18] = 2;
		gameField[6][18] = 2;
		centerCurrentFigure.x = 5;
		centerCurrentFigure.y = 19;
	} break;
	// z - obrazn flip horizontal
	case 3:
	{
		gameField[4][18] = 2;
		gameField[5][18] = 2;
		gameField[5][19] = 2;
		gameField[6][19] = 2;
		centerCurrentFigure.x = 5;
		centerCurrentFigure.y = 19;
	} break;
	// bukva G
	case 4:
	{
		gameField[4][18] = 2;
		gameField[4][19] = 2;
		gameField[5][19] = 2;
		gameField[6][19] = 2;
		centerCurrentFigure.x = 5;
		centerCurrentFigure.y = 18;
	} break;
	// bukva G flip horizontal
	case 5:
	{
		gameField[6][18] = 2;
		gameField[4][19] = 2;
		gameField[5][19] = 2;
		gameField[6][19] = 2;
		centerCurrentFigure.x = 5;
		centerCurrentFigure.y = 18;
	} break;
	// T obrazn
	case 6:
	{
		gameField[4][19] = 2;
		gameField[5][19] = 2;
		gameField[6][19] = 2;
		gameField[5][18] = 2;
		centerCurrentFigure.x = 5;
		centerCurrentFigure.y = 19;
	} break;
	invalid_default
	}
}

int CheckForPlayerFigure()
{
	for (int j = 0; j < gameFieldHeight; j++)
	{
		for (int i = 0; i < gameFieldWidth; i++)
		{
			if (gameField[i][j] == 2)
			{
				return j;
			}
		}
	}
	return -1;
}

int WhatToDoWithPlayerFigure(int lowerY)
{
	if (lowerY == 0)
	{
		return 1;
	}
	else
	{
		for (int i = 0; i < gameFieldWidth; i++)
		{
			for (int j = 1; j < gameFieldHeight; j++)
			{
				if ((gameField[i][j] == 2) && (gameField[i][j - 1] == 1))
				{
					return 1;
				}
			}
		}
	}

	return 2;
}

// just check higher line
bool CheckForGameEnd()
{
	for (int i = 0; i < gameFieldWidth; i++)
	{
		if (gameField[i][gameFieldHeight - 1] == 1)
			return true;
	}
	return false;
}

void UpdateGameTick()
{
	// 0 - reserv
	// 1 - change to 1
	// 2 - lower the figure
	int whatToDo = 0;

	int lowerY = CheckForPlayerFigure();

	whatToDo = WhatToDoWithPlayerFigure(lowerY);

	if (whatToDo == 2)
	{
		for (int i = 0; i < gameFieldWidth; i++)
		{
			for (int j = 1; j < gameFieldHeight; j++)
			{
				if (gameField[i][j] == 2)
				{
					gameField[i][j - 1] = 2;
					gameField[i][j] = 0;
				}
			}
		}
		centerCurrentFigure.y--;
	}
	else if (whatToDo == 1)
	{
		for (int i = 0; i < gameFieldWidth; i++)
		{
			for (int j = 0; j < gameFieldHeight; j++)
			{
				if (gameField[i][j] == 2)
				{
					gameField[i][j] = 1;
				}
			}
		}
	}
	else
	{
		ASSERT(whatToDo);
	}

	CheckOnBurningLine();

	if (CheckForGameEnd())
	{
		ResetField();
	}

}

bool GetMartixAroundCenter(m3* mat)
{
	if (((centerCurrentFigure.x - 1) < 0) || (centerCurrentFigure.x + 1 > (gameFieldWidth - 1))
		|| ((centerCurrentFigure.y - 1) < 0) || ((centerCurrentFigure.y + 1) > (gameFieldHeight - 1)))
	{
		return false;
	}

	mat->_11 = (f32)gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 1];
	mat->_12 = (f32)gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 1];
	mat->_13 = (f32)gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 1];
	mat->_21 = (f32)gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 0];
	mat->_22 = (f32)gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 0];
	mat->_23 = (f32)gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 0];
	mat->_31 = (f32)gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 1];
	mat->_32 = (f32)gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 1];
	mat->_33 = (f32)gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 1];

	return true;
}

void SetMatrixValueAroundCenter(m3* mat)
{
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 1] = (i32)mat->_11;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 1] = (i32)mat->_12;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 1] = (i32)mat->_13;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 0] = (i32)mat->_21;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 0] = (i32)mat->_22;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 0] = (i32)mat->_23;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 1] = (i32)mat->_31;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 1] = (i32)mat->_32;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 1] = (i32)mat->_33;
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

	for (int i = 0; i < 9; i++)
	{
		if ((mat->data[i] == 2) && (temp.data[i] == 1))
		{
			return false;
		}
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

	for (int i = 0; i < 9; i++)
	{
		if ((mat->data[i] == 2) && (temp.data[i] == 1))
		{
			return false;
		}
	}

	return true;
}

bool GetMartixAroundCenter(m4* mat)
{
	if (((centerCurrentFigure.x - 2) < 0) || (centerCurrentFigure.x + 1 > (gameFieldWidth - 1))
		|| ((centerCurrentFigure.y - 1) < 0) || ((centerCurrentFigure.y + 2) > (gameFieldHeight - 1)))
	{
		return false;
	}

	mat->_11 = (f32)gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 2];
	mat->_12 = (f32)gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 2];
	mat->_13 = (f32)gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 2];
	mat->_14 = (f32)gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 2];
	mat->_21 = (f32)gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 1];
	mat->_22 = (f32)gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 1];
	mat->_23 = (f32)gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 1];
	mat->_24 = (f32)gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 1];
	mat->_31 = (f32)gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 0];
	mat->_32 = (f32)gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 0];
	mat->_33 = (f32)gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 0];
	mat->_34 = (f32)gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 0];
	mat->_41 = (f32)gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y - 1];
	mat->_42 = (f32)gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 1];
	mat->_43 = (f32)gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 1];
	mat->_44 = (f32)gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 1];

	return true;
}

void SetMatrixValueAroundCenter(m4* mat)
{
	gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 2] = (i32)mat->_11;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 2] = (i32)mat->_12;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 2] = (i32)mat->_13;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 2] = (i32)mat->_14;
	gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 1] = (i32)mat->_21;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 1] = (i32)mat->_22;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 1] = (i32)mat->_23;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 1] = (i32)mat->_24;
	gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y + 0] = (i32)mat->_31;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y + 0] = (i32)mat->_32;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y + 0] = (i32)mat->_33;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y + 0] = (i32)mat->_34;
	gameField[centerCurrentFigure.x - 2][centerCurrentFigure.y - 1] = (i32)mat->_41;
	gameField[centerCurrentFigure.x - 1][centerCurrentFigure.y - 1] = (i32)mat->_42;
	gameField[centerCurrentFigure.x - 0][centerCurrentFigure.y - 1] = (i32)mat->_43;
	gameField[centerCurrentFigure.x + 1][centerCurrentFigure.y - 1] = (i32)mat->_44;
}

bool rotateLeft(m4* mat)
{
	m4 temp = *mat;
	*mat = Transpose(*mat);

	for (int i = 0; i < 16; i++)
	{
		if ((mat->data[i] == 2) && (temp.data[i] == 1))
		{
			return false;
		}
	}

	return true;
}

void TurnPlayerFigure(int direction)
{
	if (currentFigure > 1)
	{
		if (direction == VK_LEFT)
		{
			m3 mat = {};
			if (GetMartixAroundCenter(&mat))
			{
				if (rotateLeft(&mat))
				{
					SetMatrixValueAroundCenter(&mat);
				}
			}
		}
		else if (direction == VK_RIGHT)
		{
			m3 mat = {};
			if (GetMartixAroundCenter(&mat))
			{
				if (rotateRight(&mat))
				{
					SetMatrixValueAroundCenter(&mat);
				}
			}
		}
	}
	else if (currentFigure == 0)
	{
		m4 mat = {};
		if (GetMartixAroundCenter(&mat))
		{
			if (rotateLeft(&mat))
			{
				SetMatrixValueAroundCenter(&mat);
			}
		}
	}
}

void MovePlayerFigure(int direction)
{
	if (direction == VK_LEFT)
	{
		bool allowToMove = true;
		for (int i = 0; i < gameFieldWidth; i++)
		{
			for (int j = 0; j < gameFieldHeight; j++)
			{
				if (gameField[i][j] == 2)
				{
					if ((gameField[i - 1][j] == 1) || (i == 0))
					{
						allowToMove = false;
					}
				}
			}
		}

		if (allowToMove)
		{
			for (int i = 1; i < gameFieldWidth; i++)
			{
				for (int j = 0; j < gameFieldHeight; j++)
				{
					if (gameField[i][j] == 2)
					{
						gameField[i][j] = 0;
						gameField[i - 1][j] = 2;
					}
				}
			}
			centerCurrentFigure.x--;
		}
	}
	else if (direction == VK_RIGHT)
	{
		bool allowToMove = true;
		for (int i = 0; i < gameFieldWidth; i++)
		{
			for (int j = 0; j < gameFieldHeight; j++)
			{
				if (gameField[i][j] == 2)
				{
					if ((gameField[i + 1][j] == 1) || (i == gameFieldWidth - 1))
					{
						allowToMove = false;
					}
				}
			}
		}

		if (allowToMove)
		{
			for (int i = gameFieldWidth - 1; i >= 0; i--)
			{
				for (int j = 0; j < gameFieldHeight; j++)
				{
					if (gameField[i][j] == 2)
					{
						gameField[i][j] = 0;
						gameField[i + 1][j] = 2;
					}
				}
			}
			centerCurrentFigure.x++;
		}
	}
	else if (direction == VK_DOWN)
	{
		int lowerY = CheckForPlayerFigure();
		if (WhatToDoWithPlayerFigure(lowerY) == 2)
		{
			for (int i = 0; i < gameFieldWidth; i++)
			{
				for (int j = 1; j < gameFieldHeight; j++)
				{
					if (gameField[i][j] == 2)
					{
						gameField[i][j - 1] = 2;
						gameField[i][j] = 0;
					}
				}
			}
			centerCurrentFigure.y--;
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

void Control()
{
	if (KeyPressed(VK_LEFT))
	{
		controlLeftOnePressElapsed = 0;
		MovePlayerFigure(VK_LEFT);
	}
	if (KeyPressed(VK_RIGHT))
	{
		controlRightOnePressElapsed = 0;
		MovePlayerFigure(VK_RIGHT);
	}
	if (KeyPressed(VK_DOWN))
	{
		controlDownOnePressElapsed = 0;
		MovePlayerFigure(VK_DOWN);
	}
	if (KeyPressed(VK_UP))
	{
		TurnPlayerFigure(VK_LEFT);
	}
	if (KeyPressed(0x58)) // x
	{
		TurnPlayerFigure(VK_RIGHT);
	}
	if (KeyPressed(0x5A)) // z
	{
		TurnPlayerFigure(VK_LEFT);
	}

	if (KeyDown(VK_DOWN))
	{
		controlDownOnePressElapsed += currentFrame - lastFrame;
		if (controlDownOnePressElapsed >= controlOnePressDelay)
		{
			controlDownElapsed += currentFrame - lastFrame;
			if (controlDownElapsed >= controlDelay)
			{
				controlDownElapsed = 0;
				MovePlayerFigure(VK_DOWN);
			}
		}
	}

	if (KeyDown(VK_LEFT))
	{
		controlLeftOnePressElapsed += currentFrame - lastFrame;
		if (controlLeftOnePressElapsed >= controlOnePressDelay)
		{
			controlLeftElapsed += currentFrame - lastFrame;
			if (controlLeftElapsed >= controlDelay)
			{
				controlLeftElapsed = 0;
				MovePlayerFigure(VK_LEFT);
			}
		}
	}

	if (KeyDown(VK_RIGHT))
	{
		controlRightOnePressElapsed += currentFrame - lastFrame;
		if (controlRightOnePressElapsed >= controlOnePressDelay)
		{
			controlRightElapsed += currentFrame - lastFrame;
			if (controlRightElapsed >= controlDelay)
			{
				controlRightElapsed = 0;
				MovePlayerFigure(VK_RIGHT);
			}
		}
	}

	KeyReleased(VK_LEFT);
	KeyReleased(VK_RIGHT);
	KeyReleased(VK_DOWN);
	KeyReleased(VK_UP);
	KeyReleased(0x58);
	KeyReleased(0x5A);
}

void UpdateTetrisGame()
{
	currentFrame = GetTimeStampMilliSecond();
		timeElapsed += currentFrame - lastFrame;
		Control();
		lastFrame = currentFrame;
		if (timeElapsed >= speedMillisecond)
		{
			UpdateGameTick();
			if (CheckForPlayerFigure() < 0)
			{
				SpawnFigure();
			}
			timeElapsed = 0;
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
				u8 reserv = 0;

				*pixel++ = (blue | (green << 8) | (red << 16) | (reserv << 24));
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
				u8 reserv = 0;

				*pixel++ = (blue | (green << 8) | (red << 16) | (reserv << 24));
			}
			row += buffer->pitch;
		}
	}
}

void RenderTetrisGame(Game_Bitmap_Offscreen_Buffer* buffer)
{
	int posx = (i32)(buffer->width / 2.5f);
	int posy = 20;
	f32 widthCoef = (f32)buffer->width / 800;
	f32 heightCoef = (f32)buffer->height / 450;
	f32 scaleCoef = 20.0f;

	int border = 2;
	// drawing glass
	DrawingFillRectangle(buffer, posx, posy, (i32)(gameFieldWidth * scaleCoef * widthCoef), (i32)(gameFieldHeight * scaleCoef * heightCoef), 1);

	for (int i = 0; i < gameFieldWidth; i++)
	{
		for (int j = 0; j < gameFieldHeight; j++)
		{
			if (gameField[i][j] > 0)
			{
				DrawingFillRectangle(buffer, (i32)(i * scaleCoef * widthCoef + posx), (i32)(j * scaleCoef * heightCoef) + posy, (i32)(scaleCoef * widthCoef) - border, (i32)(scaleCoef * heightCoef) - border, 0);
			}
		}
	}
}