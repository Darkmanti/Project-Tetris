#pragma once

#define DEFAULT_BUFLEN 1024

#include "tetris.h"
#include "debug_console.h"

void GameInfoToBuffer(MPSendInfo* info, char* buffer);
void BufferToGameInfo(MPRecieveInfo* info, char* buffer);