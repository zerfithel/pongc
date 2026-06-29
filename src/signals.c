#define ENET_IMPLEMENTATION
#include "signals.h"
#include "config.h"
#include "shared.h"
#include <enet/enet.h>
#include <stdio.h>
#include <string.h>

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
      mtx_lock(&shared->mtx);
      shared->y[1] = y;
      mtx_unlock(&shared->mtx);
    }
    break;
  }
  case SIGNAL_BALL: {
    Ball ball;
    if (sscanf(message + SIGNALS[i].len, "%f,%f,%f,%f,%f", &ball.x, &ball.y,
               &ball.dx, &ball.dy, &ball.speed) == 5) {

      mtx_lock(&shared->mtx);
        shared->ball.x = ball.x;
        shared->ball.y = ball.y;
        shared->ball.dx = ball.dx;
        shared->ball.dy = ball.dy;
        shared->ball.speed = ball.speed;
      mtx_unlock(&shared->mtx);
    }
    break;
  }

  case SIGNAL_SERVER_FULL: {
    fprintf(stderr, "Room is full\n");
    break;
  }

  default: {
    fprintf(stderr, "ERROR: Invalid signal ID: %zu\n", i);
    break;
  }
  }
  return;
}

/// SIGNAL SENDERS
void send_signal_pos(ENetPeer *peer, float y) {
  char buffer[64];
  size_t buffer_size = sizeof(buffer);

  int len = snprintf(buffer, buffer_size, "pos;%f", (double)y);
  
  if (len < 0) {
    fprintf(stderr, "ERROR: Formatting pos message failed\n");
    return;
  }

  if ((size_t)len >= buffer_size) {
    fprintf(stderr, "WARNING: pos message truncated, skipping send\n");
    return;
  }

  ENetPacket *packet = enet_packet_create(buffer, strlen(buffer) + 1,
                                          ENET_PACKET_FLAG_UNSEQUENCED);
  enet_peer_send(peer, 0, packet);

  return;
}

void send_signal_ball(ENetPeer *peer, Ball *ball) {
  char buffer[64];
  size_t buffer_size = sizeof(buffer);
  float mirrored_x = LOGICAL_WIDTH - ball->x - BALL_WIDTH;

  int len = snprintf(buffer, sizeof(buffer), "ball;%f,%f,%f,%f,%f", (double)mirrored_x,
           (double)ball->y, (double)-(ball->dx), (double)ball->dy,
           (double)ball->speed);

  if (len < 0) {
    fprintf(stderr, "ERROR: Formatting ball message failed\n");
    return;
  }

  if ((size_t)len >= buffer_size) {
    fprintf(stderr, "ERROR: ball message truncated, skipping send\n");
    return;
  }

  ENetPacket *packet = enet_packet_create(buffer, strlen(buffer) + 1,
                                          ENET_PACKET_FLAG_UNSEQUENCED);
  enet_peer_send(peer, 0, packet);

  return;
}
