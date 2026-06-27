#include <stdio.h>
#include <string.h>

#include "cmdline.h"

/*
 * Find option from given arg and return its index
 */
static const Option *find_option(char *arg, const Option *options,
                                 int options_count) {
  for (int i = 0; i < options_count; i++) {

    // --long
    if (arg[0] == '-' && arg[1] == '-' &&
        strcmp(arg + 2, options[i].long_opt) == 0) {
      return &options[i];
    }

    // -s
    if (arg[0] == '-' && arg[1] != '-' && arg[1] == options[i].short_opt) {
      return &options[i];
    }
  }

  return NULL;
}

/*
 * Main argument parser
 * Writes out data (args) to *out
 * Returned value is status
 */
int parse_args(Args *out, int argc, char **argv, const Option *options,
               int options_count) {
  if (!out || !argv || !options) {
    return -1;
  }

  for (int i = 1; i < argc; i++) {

    const Option *opt = find_option(argv[i], options, options_count);

    if (!opt) {
      fprintf(stderr, "Unknown option: %s\n", argv[i]);
      return 1;
    }

    const char *value = NULL;

    if (opt->argument) {
      if (i + 1 < argc) {
        value = argv[++i];
      } else {
        // fprintf(stderr, "Missing argument for %s\n", argv[i]);
        continue;
      }
    }

    int ret = opt->handler(out, value);
    if (ret != 0)
      return ret;
  }

  return 0;
}
