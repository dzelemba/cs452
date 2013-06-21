#include "test_helpers.h"
#include "syscall.h"
#include "strings.h"
#include "stdio.h"
#include "kernel.h"

static int failure;

/*
 * Private Methods
 */

void print_message(char* message) {
  if (message) {
    printf(COM2, "%s : ", message);
  }
}

/* Public Methods */

void start_test(char* name) {
  // If we want to print "Start test" or something later..
  (void)name; // Silence compiler warnings

  init_kernel();
  reset_did_fail();
}

void end_test(char* name) {
  if (did_fail()) {
    printf(COM2, "%s Failed!\n", name);
  } else {
    printf(COM2, "%s Passed!\n", name);
  }
}

void reset_did_fail() {
  failure = 0;
}

void record_failure() {
  failure = 1;
}

int did_fail() {
  return failure;
}

void assert_string_equals(char* a, char* b, char* message) {
  if (!string_equal(a, b)) {
    print_message(message);
    printf(COM2, "Expected %s but got %s \n", a, b);
    record_failure();
  }
}

void assert_char_equals(char a, char b, char* message) {
  if (a != b) {
    print_message(message);
    printf(COM2, "Expected (%c, %x) but got (%c, %x) \n", a, a, b, b);
    record_failure();
  }
}

void assert_int_equals(int a, int b, char* message) {
  if (a != b) {
    print_message(message);
    printf(COM2, "Expected %d but got %d \n", a, b);
    record_failure();
  }
}

void assert_no_error(char* error, char* message) {
  if (error) {
    print_message(message);
    printf(COM2, "Unexpected error: %s\n", error);
    record_failure();
  }
}

void assert_error(char* error, char* message) {
  if (!error) {
    print_message(message);
    printf(COM2, "Expected error but got null\n");
    record_failure();
  }
}

void assert_true(int boolean, char* message) {
  if (!boolean) {
    print_message(message);
    printf(COM2, "Expression not true\n");
    record_failure();
  }
}

void assert_false(int boolean, char* message) {
  if (boolean) {
    print_message(message);
    printf(COM2, "Expression not false\n");
    record_failure();
  }
}
