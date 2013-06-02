#include "debug.h"
#include <bwio.h>

static int method_depth = 0;

void print_depth_space() {
  int d;
  for (d = 0; d < method_depth; d++) {
    bwputstr(COM2, "  ");
  }
}

void print_debug(char* format, ...) {
  va_list args;

  va_start(args, format);
  bwformat(COM2, format, args);
  va_end(args);
}

void method_entry(char* format, ...) {
  print_depth_space();
  bwputstr(COM2, "-->");

  va_list args;

  va_start(args, format);
  bwformat(COM2, format, args);
  va_end(args);

  method_depth++;
}

void method_exit(char* format, ...) {
  method_depth--;
  print_depth_space();
  bwputstr(COM2, "<--");

  va_list args;

  va_start(args, format);
  bwformat(COM2, format, args);
  va_end(args);
}

void assert(int exp, char* exp_str, char* msg) {
  if (!exp) {
    bwprintf(COM2, "Assertion Failed! (%s) , %s\n", exp_str, msg);
  }
}
