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

#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)

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

struct Game_Memory
{
    bool isInitialized;

    u64 permanentStorageSize;
    void* permanentStorage; // In WIndows this initialize by ZERO, but on particular platform this may not be

    u64 transientStorageSize;
    void* transientStorage;
};

struct Game_State
{
    int xOffset;
    int yOffset;
    int toneHz;
};

bool keyState[256] = {};

i64 controlDownOnePressElapsed = 0;
i64 controlDownElapsed = 0;
i64 controlLeftOnePressElapsed = 0;
i64 controlLeftElapsed = 0;
i64 controlRightOnePressElapsed = 0;
i64 controlRightElapsed = 0;

i64 controlOnePressDelay = 130;
i64 controlDelay = 90;

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

struct Tetris_Game_Field
{
    int width;
    int height;
    int** data;
};

struct Tetris_Game_State
{
    Tetris_Game_Field field;

    i64 lastFrame;
    i64 currentFrame;
    i64 timeElapsed;
    i64 speedMillisecond;

    int currentFigure;
    Game_Point centerCurrentFigure;

    int nextFigure;

    int maxScore;
    int currentScore;
};

struct MPRecieveInfo
{
    Tetris_Game_Field* field;
    int currentScore;
    int maxScore;
};

struct MPSendInfo
{
    Tetris_Game_Field* field;
    int* currentScore;
    int* maxScore;
};

Tetris_Game_State hostGameState = {};

void GameUpdateAndRender(Game_Memory* memory, Game_Input* input, Game_Bitmap_Offscreen_Buffer* buffer, Game_Sound_Output_Buffer* soundBuffer, i64 currentFrame, MPRecieveInfo* info);

void RenderTetrisGame(Game_Bitmap_Offscreen_Buffer* buffer, int posx, Tetris_Game_Field* field);
void UpdateTetrisGame(Tetris_Game_State* gameState, i64 currentFrame);

void InitTetrisGame(Tetris_Game_State* gameState);
void InitGameField(Tetris_Game_Field* field);

void ClearInput(IPStrucrute* ipStruct);