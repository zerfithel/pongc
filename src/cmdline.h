#pragma once

#include <enet/enet.h>
#include <stdbool.h>

/*
 * Program arguments which values are returned by handlers
 */
typedef struct {
  char ip[64];
  enet_uint16 port;
  bool is_server;
} Args;

/*
 * Single option structure for cmdline parser
 * Calls handler(value) with argument value, if argument == false, then value is
 * NULL
 */
typedef struct {
  int (*handler)(Args *args, const char *);
  char *long_opt;
  char short_opt;
  bool argument;
} Option;

/*
 * Main argument parser
 * Writes out data (args) to *out
 * Returned value is status
 */
int parse_args(Args *out, int argc, char **argv, const Option *options,
               int options_count);
