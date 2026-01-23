#pragma once
#include "shared.h"
#include "utils.h" // Vec2

/// Signal message structures

/*
typedef struct {
  unsigned int me, him;
} Score;
*/

typedef enum {
  SIGNAL_POS,
  SIGNAL_BALL,

  SIGNALS_COUNT // Count of all signals
} SignalType;

// Signals handler
void handle_signal(SharedData *shared, char *message);

// Signal senders
void send_signal_pos(ENetPeer *peer, float y);       // pos;y
void send_signal_ball(ENetPeer *peer, Vec2 v);      // ball;x,y

typedef struct {
  const char *msg; // Prefix
  size_t len;      // Length of prefix 
} Signal;

// Prefix for each signal and its length
static const Signal SIGNALS[SIGNALS_COUNT] =
{
  { 
    .msg = "pos;", 
    .len = 4 
  },
  { 
    .msg = "ball;", 
    .len = 5 
  },
};
