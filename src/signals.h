#pragma once
#include <enet/enet.h>

#include "ball.h"
#include "shared.h"

typedef enum {
  SIGNAL_POS,
  SIGNAL_BALL,
  SIGNAL_SERVER_FULL,

  SIGNALS_COUNT // Count of all signals
} SignalType;

// Signals handler
void handle_signal(SharedData *shared, char *message);

// Signal senders
void send_signal_pos(ENetPeer *peer, float y);     // pos;y
void send_signal_ball(ENetPeer *peer, Ball *ball); // ball;x,y

typedef struct {
  const char *msg; // Prefix
  size_t len;      // Length of prefix
} Signal;

// Prefix for each signal and its length
static const Signal SIGNALS[SIGNALS_COUNT] = {
    {.msg = "pos;", .len = 4},
    {.msg = "ball;", .len = 5},
    {.msg = "server_full", .len = 11}};
