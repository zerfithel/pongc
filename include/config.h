#pragma once
#include <SDL2/SDL.h>

// Game settings
#define GAME_TPS             50.0f

// Paddle settings
#define PADDLE_WIDTH         10
#define PADDLE_HEIGHT        200
#define PADDLE_SPEED         300.0f

// Ball settings
#define BALL_WIDTH           25
#define BALL_HEIGHT          25
#define BALL_SPEED           890.0f
#define BALL_START_SPEED     220.0f
#define BALL_SPEED_INCREASE  10.0f

// SDL/OpenGL
#define WINDOW_FLAGS SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP

#define LOGICAL_WIDTH        1280
#define LOGICAL_HEIGHT       720
#define WINDOW_WIDTH         1280
#define WINDOW_HEIGHT        720
