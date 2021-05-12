#pragma once

#define DEFAULT_BUFLEN 1024

#include "tetris.h"

void GameFieldToBuffer(GameField* field, char* buffer);
void BufferToGameField(GameField* field, char* buffer);