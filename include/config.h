#pragma once
#include <SDL2/SDL.h>

// debug messages
// do not use as this literally does nothing, for debugging i started using debugger (gdb)
// because game is multithreaded
#define DEBUG_LOG false

// Game settings
constexpr float GAME_TPS      = 50.0f;
constexpr float PADDLE_SPEED  = 300.0f;
constexpr int   PADDLE_WIDTH  = 10;
constexpr int   PADDLE_HEIGHT = 200;
constexpr int   BALL_WIDTH    = 25;
constexpr int   BALL_HEIGHT   = 25;
constexpr float BALL_SPEED    = 890.0f;
constexpr float BALL_START_SPEED = 220.0f;

// SDL2
constexpr int WINDOW_FLAGS   = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP;
constexpr int RENDERER_FLAGS = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

constexpr int LOGICAL_WIDTH  = 1280;
constexpr int LOGICAL_HEIGHT = 720;
