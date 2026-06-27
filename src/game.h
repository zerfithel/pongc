#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "shared.h"

/*
 * Main thread game loop
 * Is responsible for ball logic, player movement, input, render
 */
void game_loop(SDL_Window *window, SharedData *shared, bool server);
