#include "test_helpers.h"
#include <bwio.h>
#include "syscall.h"

static int failure;

/*
 * Private Methods
 */

void print_message(char* message) {
	if (message) {
		bwprintf(COM2, "%s : ", message);
	}
}

/* Public Methods */

void reset_did_fail() {
  failure = 0;
}

void record_failure() {
  failure = 1;
}

int did_fail() {
  return failure;
}

void assert_equals(char a, char b, char* message) {
	if (a != b) {
		print_message(message);
		bwprintf(COM2, "Expected (%c, %x) but got (%c, %x) \n", a, a, b, b);
    record_failure();
		Exit();
	}
}

void assert_int_equals(int a, int b, char* message) {
	if (a != b) {
		print_message(message);
		bwprintf(COM2, "Expected %d but got %d \n", a, b);
    record_failure();
		Exit();
	}
}

void assert_no_error(char* error, char* message) {
	if (error) {
		print_message(message);
		bwprintf(COM2, "Unexpected error: %s\n", error);
    record_failure();
		Exit();
	}
}

void assert_error(char* error, char* message) {
	if (!error) {
		print_message(message);
		bwprintf(COM2, "Expected error but got null\n");
    record_failure();
		Exit();
	}
}

void assert_true(int boolean, char* message) {
	if (!boolean) {
		print_message(message);
		bwprintf(COM2, "Expression not true\n");
    record_failure();
		Exit();
	}
}

void assert_false(int boolean, char* message) {
	if (boolean) {
		print_message(message);
		bwprintf(COM2, "Expression not false\n");
    record_failure();
		Exit();
	}
}
