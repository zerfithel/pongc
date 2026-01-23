#pragma once
#include <enet/enet.h>

int host_server(enet_uint16 port);
int server_loop(void *data);
