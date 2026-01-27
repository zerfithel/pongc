#pragma once
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "shared.h"

void game_loop(SDL_Window *window, SharedData *shared, bool server);
