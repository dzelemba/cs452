#include "debug.h"
#include "ourio.h"
#include "interrupt_handler.h"
#include "user_prompt.h"

static int method_depth = 0;

void print_depth_space() {
  int d;
  for (d = 0; d < method_depth; d++) {
    putstr(COM2, "  ");
  }
}

void print_debug(char* format, ...) {
  va_list args;

  va_start(args, format);
  io_format(COM2, format, args);
  va_end(args);
}

void method_entry(char* format, ...) {
  print_depth_space();
  putstr(COM2, "-->");

  va_list args;

  va_start(args, format);
  io_format(COM2, format, args);
  va_end(args);

  method_depth++;
}

void method_exit(char* format, ...) {
  method_depth--;
  print_depth_space();
  putstr(COM2, "<--");

  va_list args;

  va_start(args, format);
  io_format(COM2, format, args);
  va_end(args);
}

void assert(int exp, char* exp_str, char* format, ...) {
  if (!exp) {
    reset_interrupts();
    bwprintf(COM2, "Assertion Failed! (%s) ", exp_str);

    va_list args;
    va_start(args, format);
    bwformat(COM2, format, args);
    va_end(args);
    bwprintf(COM2, "\n");
  }
}

void error(char* format, ...) {
  reset_interrupts();

  bwprintf(COM2, "ERROR: ");

  va_list args;
  va_start(args, format);
  bwformat(COM2, format, args);
  va_end(args);
}

// I'm hoping the consts will make the compiler do smart things here.
static const int const enabled_groups [NUM_DEBUG_GROUPS] = {
  0, /* TRAIN_CONTROLLER */
  0, /* LOCATION_SERVER */
  0, /* RESERVATION_SERVER */
  0 /* DEMO */
  0, /* MISC */
  0, /* MISC2 */
};

void info(debug_group group, char* format, ...) {
  if (enabled_groups[group]) {
    va_list args;
    va_start(args, format);
    format_debug_output(format, args);
    va_end(args);
  }
}
