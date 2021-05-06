#pragma once
#include "math.h"

/*
  TODO: Services that the platform layer provides to the game
*/

/*
  NOTE: Services that the game provides to the platform layer.
  (this may expand in the future - sound on separate thread, etc.)
*/

// FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

// TODO: In the future, rendering _specifically_ will become a three-tiered abstraction!!!

struct Game_Sound_Output_Buffer
{
    int samplesPerSecond;
    int sampleCount;
    i16* samples;
};

struct Game_Bitmap_Offscreen_Buffer
{
    // pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    void* memory;
    int width;
    int height;
    int pitch;
};

// TODO: Escaping problem
#include "font_proccesing.h"

struct Game_Point
{
    int x;
    int y;
};

struct Game_Input
{

};

bool keyState[256] = {};

const int gameFieldWidth = 10;
const int gameFieldHeight = 20;
int gameField[gameFieldWidth][gameFieldHeight]{};

i64 lastFrame = 0;
i64 currentFrame = 0;
i64 timeElapsed = 0;
i64 speedMillisecond = 400;

i64 controlDownOnePressElapsed = 0;
i64 controlDownElapsed = 0;
i64 controlLeftOnePressElapsed = 0;
i64 controlLeftElapsed = 0;
i64 controlRightOnePressElapsed = 0;
i64 controlRightElapsed = 0;

i64 controlOnePressDelay = 450;
i64 controlDelay = 60;

int currentFigure = -1;
Game_Point centerCurrentFigure = { -1 };

Font font = {};

// TODO: Just testing
struct IPStrucrute
{
    wchar_t ipString[32];
    int ipLength;
};

IPStrucrute ipStructure = {};

void ProccesIPInput(IPStrucrute* ip);

// TODO: GameState
int gameState = 1;

void GameUpdateAndRender(Game_Input* input, Game_Bitmap_Offscreen_Buffer* buffer, Game_Sound_Output_Buffer* soundBuffer);

void RenderTetrisGame(Game_Bitmap_Offscreen_Buffer* buffer);
void UpdateTetrisGame();