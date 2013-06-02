#include "test_helpers.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Private Methods
 */
void print_message(char* message) {
	if (message) {
		printf("%s : ", message);
	}
}

/* Public Methods */

void assert_equals(char a, char b, char* message) {
	if (a != b) {
		print_message(message);
		printf("Expected (%c, %x) but got (%c, %x) \n", a, a, b, b);
		exit(-1);
	}
}

void assert_int_equals(int a, int b, char* message) {
	if (a != b) {
		print_message(message);
		printf("Expected %d but got %d \n", a, b);
		exit(-1);
	}
}

void assert_no_error(char* error, char* message) {
	if (error) {
		print_message(message);
		printf("Unexpected error: %s\n", error);
		exit(-1);
	}
}

void assert_error(char* error, char* message) {
	if (!error) {
		print_message(message);
		printf("Expected error but got null\n");
		exit(-1);
	}
}

void assert_true(int boolean, char* message) {
	if (!boolean) {
		print_message(message);
		printf("Expression not true\n");
		exit(-1);
	}
}

void assert_false(int boolean, char* message) {
	if (boolean) {
		print_message(message);
		printf("Expression not false\n");
		exit(-1);
	}
}
