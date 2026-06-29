#include <enet/enet.h>

#include "random.h"
#include "ball.h"
#include "config.h"
#include "game.h"
#include "server.h"
#include "signals.h"
#include "utils.h"

static ENetPeer *client_peer = NULL;
static ENetHost *server_host = NULL;
static bool slot_taken = false; // is player slot taken?

int host_server(const char *ip, enet_uint16 port) {
  ENetAddress address;
  address.port = port;

  if (!ip) {
    address.host = ENET_HOST_ANY; // fallback
  } else {
    if (enet_address_set_host(&address, ip) != 0) {
      return 1; // DNS resolve fail / invalid IP
    }
  }

  server_host = enet_host_create(&address, 2, 2, 0, 0);
  if (server_host == NULL) {
    return 1;
  }

  return 0;
}

int server_loop(void *data) {
  uint32_t seed = make_seed();

  SharedData *shared = (SharedData *)data;
  ENetEvent event;

  const double tick_dt = 1.0 / NET_TPS;
  Uint64 prev_counter = SDL_GetPerformanceCounter();
  double accumulator = 0.0;

  float last_sent_y = 0.0f;
  float last_ball_dx = -1.0f;
  float last_ball_dy = -1.0f;

  while (atomic_load(&shared->running)) {
    // time
    Uint64 now = SDL_GetPerformanceCounter();
    double frame_time =
        (double)(now - prev_counter) / (double)SDL_GetPerformanceFrequency();
    prev_counter = now;

    if (frame_time > 0.25) {
      frame_time = 0.25;
    }
    accumulator += frame_time;

    // non blocking receive
    while (enet_host_service(server_host, &event, 0) > 0) {
      switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT: {
        mtx_lock(&shared->mtx);

        // slot is not taken
        if (!slot_taken) {
          shared->y[0] = LOGICAL_HEIGHT >> 1;
          shared->y[1] = LOGICAL_HEIGHT >> 1;

          printf("Info: Client joined: %x:%u\n", event.peer->address.host,
                 event.peer->address.port);

          slot_taken = true;
          client_peer = event.peer;
          last_sent_y = shared->y[0];

          shared->ball.x = LOGICAL_WIDTH >> 1;
          shared->ball.y = LOGICAL_HEIGHT >> 1;
          shared->ball.dx = (rand() % 2) ? 1.0f : -1.0f;
          shared->ball.dy = rand_range(-0.5f, 0.5f, seed);
          normalize2f(&shared->ball.dx, &shared->ball.dy);
          shared->ball.speed = BALL_START_SPEED;
        } else {
          printf("Info: %x:%u tried to connect, but player slot is already "
                 "taken\n",
                 event.peer->address.host, event.peer->address.port);

          char message[16];
          size_t message_size = sizeof(message);
          int len = snprintf(message, message_size, "%s", "server_full");
          
          if (len < 0) {
            fprintf(stderr, "WARNING: Formatting server_full message failed, skipping...\n");
            goto send_cleanup;
          }

          if ((size_t)len >= message_size) {
            fprintf(stderr, "WARNING: server_full message truncated, skipping...\n");
            goto send_cleanup;
          }

          ENetPacket *packet = enet_packet_create(message, strlen(message) + 1,
                                                  ENET_PACKET_FLAG_RELIABLE);

          enet_peer_send(event.peer, 0, packet);
          enet_host_flush(server_host);
        }

send_cleanup:
        mtx_unlock(&shared->mtx);
        break;
      }

      case ENET_EVENT_TYPE_DISCONNECT: {
        mtx_lock(&shared->mtx);

        printf("Info: Client disconnected\n");

        fflush(stdout);
        slot_taken = false;
        client_peer = NULL;
        shared->y[1] = 0.0f;

        mtx_unlock(&shared->mtx);
        break;
      }

      case ENET_EVENT_TYPE_RECEIVE: {
        if (event.peer != client_peer) {
          fprintf(stderr, "ERROR: Received message from different peer, "
                          "ignoring packet...\n");
          goto receive_cleanup;
        }

        char buffer[64];
        size_t len = event.packet->dataLength;

        if (len >= sizeof(buffer)) {
          fprintf(stderr,
                  "WARNING: Received too much data, ignoring packet...\n");
          goto receive_cleanup;
        }

        memcpy(buffer, event.packet->data, len);
        buffer[len] = '\0';

        handle_signal(shared, buffer);

receive_cleanup:
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

      mtx_lock(&shared->mtx);
      local_y[0] = shared->y[0];
      local_y[1] = shared->y[1];
      mtx_unlock(&shared->mtx);

      // Send new position to player and moving vector to player, if ball
      // changed move direction
      if (slot_taken && ( !float_equal(shared->ball.dx, last_ball_dx) ||
                          !float_equal(shared->ball.dy, last_ball_dy) )) {

        Ball ball;
        ball = shared->ball;
        send_signal_ball(client_peer, &ball);

        last_ball_dx = shared->ball.dx;
        last_ball_dy = shared->ball.dy;
      }

      if (slot_taken && client_peer) {
        float current_y = local_y[0];

        if (current_y != last_sent_y) {
          send_signal_pos(client_peer, current_y);
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
