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

i64 controlDownOnePressElapsed = 0;
i64 controlDownElapsed = 0;
i64 controlLeftOnePressElapsed = 0;
i64 controlLeftElapsed = 0;
i64 controlRightOnePressElapsed = 0;
i64 controlRightElapsed = 0;

i64 controlOnePressDelay = 60;
i64 controlDelay = 40;

Font font = {};

bool gameInit = false;

// TODO: Just testing
struct IPStrucrute
{
    wchar_t ipString[32];
    int ipLength;
};

IPStrucrute ipStructure = {};

void ProccesIPInput(IPStrucrute* ip);

// TODO: GameState
int multiplayerState = 0;
bool tryToConnectClient = false;
bool listenConnectToServer = true;
bool connectionEstablished = false;
bool allowToSend = false;
bool allowToCreateRecieveThread = false;
bool runMPGame = false;

struct GameField
{
    int width;
    int height;
    int** data;
};

struct TetrisHostGameState
{
    GameField field;

    i64 lastFrame;
    i64 currentFrame;
    i64 timeElapsed;
    i64 speedMillisecond;

    int currentFigure;
    Game_Point centerCurrentFigure;

    int nextFigure;
};

struct TetrisClientGameState
{
    GameField field;
};

TetrisHostGameState hostGameState = {};
TetrisClientGameState clientGameState = {};

void GameUpdateAndRender(Game_Input* input, Game_Bitmap_Offscreen_Buffer* buffer, Game_Sound_Output_Buffer* soundBuffer, i64 currentFrame, GameField* rivalField);

void RenderTetrisGame(Game_Bitmap_Offscreen_Buffer* buffer, int posx, GameField* field);
void UpdateTetrisGame(TetrisHostGameState* gameState, i64 currentFrame);

void InitTetrisGame(TetrisHostGameState* gameState);
void InitGameField(GameField* field);