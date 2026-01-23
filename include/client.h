#pragma once
#include <enet/enet.h>
#include "game.h"

int join_server(const char *ip, enet_uint16 port);
int client_loop(void *data);
