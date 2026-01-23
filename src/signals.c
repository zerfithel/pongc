#define ENET_IMPLEMENTATION
#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include "shared.h"
#include "signals.h"

// Handle signals
void handle_signal(SharedData *shared, char *message) {
  size_t i = 0;

  for (; i < SIGNALS_COUNT; i++) {
    if (strncmp(message, SIGNALS[i].msg, SIGNALS[i].len) == 0) {
      break;
    }
  }

  switch (i) {
    case SIGNAL_POS: {
      float y;
      if (sscanf(message + SIGNALS[i].len, "%f", &y) == 1) {
        mtx_lock(&shared->players_mtx);
        {
          shared->y[1] = y;
        }
        mtx_unlock(&shared->players_mtx);
      }
      break;
    }
    case SIGNAL_BALL: {
      Vec2 v;
      if (sscanf(message + SIGNALS[i].len, "%f,%f", &v.x, &v.y) == 2) {
        mtx_lock(&shared->ball_mtx);
        {
          shared->ball.x = v.x;
          shared->ball.y = v.y;
        }
        mtx_unlock(&shared->ball_mtx);
      }
      break;
    }

    default: {
      fprintf(stderr, "ERROR: Invalid signal ID: %zu\n", i);
    }
  }
  return;
}

/// SIGNAL SENDERS
void send_signal_pos(ENetPeer *peer, float y) {
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "pos;%f", y);
  buffer[sizeof(buffer) - 1] = '\0';

  ENetPacket *packet = enet_packet_create(
    buffer, 
    strlen(buffer) + 1, 
    ENET_PACKET_FLAG_UNSEQUENCED
  );
  enet_peer_send(peer, 0, packet);

  return;
}

void send_signal_ball(ENetPeer *peer, Vec2 v) {
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "ball;%f,%f", v.x, v.y);
  buffer[sizeof(buffer) - 1] = '\0';

  ENetPacket *packet = enet_packet_create(
    buffer, 
    strlen(buffer) + 1, 
    ENET_PACKET_FLAG_UNSEQUENCED
  );
  enet_peer_send(peer, 0, packet);

  return;
}
