/*
The MIT License (MIT)

Copyright © 2026 Zerfithel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
#include "signals.h"
#include "game.h"
#include "shared.h"
#include "config.h"

ENetHost *client_host = NULL;
ENetPeer *server_peer = NULL;

int join_server(const char *ip, enet_uint16 port) {
  ENetAddress address;
  ENetEvent event;

  client_host = enet_host_create(
    NULL,
    1,
    2,
    0, 0
  );

  if (client_host == NULL) {
    fprintf(stderr, "ERROR: Failed to create enet client_host host\n");
    return 1;
  }

  enet_address_set_host(&address, ip);
  address.port = port;

  server_peer = enet_host_connect(client_host, &address, 2, 0);
  if (server_peer == NULL) {
    fprintf(stderr, "ERROR: Failed to create server_peer peer\n");
    enet_host_destroy(client_host);
    client_host = NULL;
    return 1;
  }

  if (enet_host_service(client_host, &event, 5000) > 0 &&
      event.type == ENET_EVENT_TYPE_CONNECT)
  {
    printf("Info: Connected to server %s:%d\n", ip, port);
    //server_peer = event.peer;
  } else {
    fprintf(stderr, "ERROR: Failed to connect to server_peer %s:%d\n", ip, port);
    enet_peer_reset(server_peer);
    server_peer = NULL;
    enet_host_destroy(client_host);
    client_host = NULL;
    return 1;
  }

  return 1;
}

// loop ran in network thread
int client_loop(void *data) {
  SharedData *shared = data;
  ENetEvent event;

  const double tick_dt = 1.0 / GAME_TPS;
  Uint64 prev_counter = SDL_GetPerformanceCounter();
  double accumulator = 0.0;
  float last_sent_y = 0.0f;

  mtx_lock(&shared->players_mtx);
  {
    shared->y[0] = LOGICAL_HEIGHT >> 1;
    shared->y[1] = LOGICAL_HEIGHT >> 1;
    last_sent_y = shared->y[0];
  }
  mtx_unlock(&shared->players_mtx);

  while (atomic_load(&shared->running)) {
    // time
    Uint64 now = SDL_GetPerformanceCounter();
    double frame_time = (double)(now - prev_counter) / SDL_GetPerformanceFrequency();
    prev_counter = now;
    
    if (frame_time > 0.25) {
      frame_time = 0.25;
    }
    accumulator += frame_time;

    // non blocking receive
    while (enet_host_service(client_host, &event, 0) > 0) {
      if (event.peer == NULL) {
        break;
      }
      switch (event.type)
      {
        case ENET_EVENT_TYPE_CONNECT: {
          printf("Info: Connected to server_peer\n");
          break;
        }

        case ENET_EVENT_TYPE_RECEIVE: {
          if (event.peer != server_peer) {
            fprintf(stderr, "ERROR: Received message from different peer, ignoring packet...\n");
            enet_packet_destroy(event.packet);
            break;
          }
          char buffer[64];
          size_t len = event.packet->dataLength;
          if (len >= sizeof(buffer)) {
            fprintf(stderr, "WARNING: Received too much data, ignoring packet...\n");
            enet_packet_destroy(event.packet);
            break;
          }
          memcpy(buffer, event.packet->data, len);
          buffer[len] = '\0';

          handle_signal(shared, buffer);

          enet_packet_destroy(event.packet);
          break;
        }

        case ENET_EVENT_TYPE_DISCONNECT: {
          printf("Info: Disconnected from server\n");
          server_peer = NULL;
          return 0;
        }

        default: {
          break;
        }
      }
    }
    
    while (accumulator >= tick_dt) {
      float current_y;
      mtx_lock   (&shared->players_mtx);
      {
        current_y = shared->y[0];
      }
      mtx_unlock(&shared->players_mtx);

      // send position update in ticks
      if (current_y != last_sent_y && server_peer) {
        char message[64];
        int len = snprintf(message, sizeof(message), "pos;%f", current_y);
        if (len > 0) {
          ENetPacket *packet = enet_packet_create(
            message,
            (size_t)len + 1,
            ENET_PACKET_FLAG_UNSEQUENCED
          );
          enet_peer_send(server_peer, 0, packet);
        }
        last_sent_y = current_y;
      }
      accumulator -= tick_dt;
    }
    enet_host_flush(client_host);
  }

  if (server_peer != NULL) {
    enet_peer_disconnect(server_peer, 0);
    server_peer = NULL;
  }

  if (client_host != NULL) {
    enet_host_destroy(client_host);
    client_host = NULL;
  }

  return 0;
}
