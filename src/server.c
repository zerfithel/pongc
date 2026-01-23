/*
The MIT License (MIT)

Copyright © 2026 Zerfithel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <enet/enet.h>
#include "game.h"
#include "ball.h"
#include "signals.h"
#include "config.h"

ENetPeer *client_peer = NULL;
ENetHost *server_host = NULL;
bool slot_taken  = false; // is player slot taken?

int host_server(enet_uint16 port) {
  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = port;

  server_host = enet_host_create(
    &address,
    2,
    2,
    0,
    0
  );
  if (server_host == NULL) {
    fprintf(stderr, "ERROR: Failed to host server on port: %u\n", port);
    return 1;
  }

  printf("Info: Server is listening on port %u\n", port);
  return 0;
}

int server_loop(void *data) {
  SharedData *shared = (SharedData *)data;
  ENetEvent event;

  const double tick_dt = 1.0 / GAME_TPS;
  Uint64 prev_counter = SDL_GetPerformanceCounter();
  double accumulator = 0.0;

  float last_sent_y = 0.0f;
  float last_ball_x = -1.0f;
  float last_ball_y = -1.0f;

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
    while (enet_host_service(server_host, &event, 0) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
          mtx_lock(&shared->players_mtx);
          {
            shared->y[0] = LOGICAL_HEIGHT >> 1;
            shared->y[1] = LOGICAL_HEIGHT >> 1;
          }
          mtx_unlock(&shared->players_mtx);

          // slot is not taken
          if (!slot_taken) {
            printf(
              "Info: Client joined: %x:%u\n",
              event.peer->address.host,
              event.peer->address.port
            );
            slot_taken = true;
            client_peer = event.peer;
            last_sent_y = shared->y[0];
            mtx_lock  (&shared->ball_mtx);
            {
              shared->ball.x = LOGICAL_WIDTH  >> 1;
              shared->ball.y = LOGICAL_HEIGHT >> 1;
              shared->ball.dx = (rand() % 2) ? 1.0f : -1.0f;
              shared->ball.dy = (rand() % 2) ? 1.0f : -1.0f;
              shared->ball.speed = BALL_START_SPEED;
            }
            mtx_unlock(&shared->ball_mtx);
          } else {
            printf(
              "Info: %x:%u tried to connect, but player slot is already taken\n",
              event.peer->address.host,
              event.peer->address.port
            );

            char message[16];
            strncpy(message, "server_full", sizeof(message));
            ENetPacket *packet = enet_packet_create(
              message,
              strlen(message) + 1,
              ENET_PACKET_FLAG_RELIABLE
            );

            enet_peer_send(event.peer, 0, packet);
            enet_host_flush(server_host);
          }
          break;
        }

        case ENET_EVENT_TYPE_DISCONNECT: {
          printf("Info: Client disconnected\n");
          slot_taken = false;
          mtx_lock(&shared->players_mtx);
          {
            client_peer = NULL;
            shared->y[1] = 0.0f;
          }
          mtx_unlock(&shared->players_mtx);
          break;
        }

        case ENET_EVENT_TYPE_RECEIVE: {
          if (event.peer != client_peer) {
            fprintf(
              stderr,
              "ERROR: Received message from different peer, ignoring packet...\n"
            );
            enet_packet_destroy(event.packet);
            break;
          }

          char buffer[64];
          size_t len = event.packet->dataLength;
          if (len >= sizeof(buffer)) {
            fprintf(
              stderr,
              "WARNING: Received too much data, ignoring packet...\n"
            );
            enet_packet_destroy(event.packet);
            break;
          }

          memcpy(buffer, event.packet->data, len);
          buffer[len] = '\0';

          handle_signal(shared, buffer);

          enet_packet_destroy(event.packet);
          break;
        }

        default: {
          break;
        }
      }
    }

    while (accumulator >= tick_dt) {
      accumulator -= tick_dt;

      float local_y[2];
      mtx_lock(&shared->players_mtx);
      {
        local_y[0] = shared->y[0];
        local_y[1] = shared->y[1];
      }
      mtx_unlock(&shared->players_mtx);

      float local_ball_x;
      float local_ball_y;
      mtx_lock(&shared->ball_mtx);
      {
        local_ball_x = shared->ball.x;
        local_ball_y = shared->ball.y;
      }
      mtx_unlock(&shared->ball_mtx);

      // send new ball pos to player
      if (slot_taken &&
          (local_ball_x != last_ball_x || local_ball_y != last_ball_y)) {
        // mirrored x (because "ME" is always on left)
        float mirrored_x = LOGICAL_WIDTH - local_ball_x - BALL_WIDTH;

        char buf[64];
        snprintf(buf, sizeof(buf), "ball;%f,%f", mirrored_x, local_ball_y);
        buf[sizeof(buf) - 1] = '\0';
        ENetPacket *packet = enet_packet_create(
          buf,
          strlen(buf) + 1,
          ENET_PACKET_FLAG_UNSEQUENCED
        );
        enet_peer_send(client_peer, 0, packet);

        last_ball_x = local_ball_x;
        last_ball_y = local_ball_y;
      }

      // Calculate new ball pos
      // and if someone scored, increase score
      mtx_lock(&shared->ball_mtx);
      {
        // NOT SYNCHRONISED YET, NEXT UPDATE WILL HAVE IT
        int scorer = update_ball(&shared->ball, local_y, tick_dt);
        if (scorer != -1) {
          mtx_lock(&shared->score_mtx);
          {
            shared->score[scorer] += 1;
          }
          mtx_unlock(&shared->score_mtx);
        }
      }
      mtx_unlock(&shared->ball_mtx);

      if (slot_taken && client_peer) {
        float current_y = local_y[0];
        if (current_y != last_sent_y) {
          // send authorative pos
          char message[64];
          int len = snprintf(
            message,
            sizeof(message),
            "pos;%f",
            current_y
          );
          if (len > 0) {
            ENetPacket *packet = enet_packet_create(
              message,
              (size_t)len + 1,
              ENET_PACKET_FLAG_UNSEQUENCED
            );

            enet_peer_send(client_peer, 0, packet);
          }
          last_sent_y = current_y;
        }
      }
    }
  }

  enet_host_flush(server_host);
  if (server_host != NULL) {
    enet_host_destroy(server_host);
    server_host = NULL;
  }
  return 0;
}
