#ifndef __TEST_HELPERS_H__
#define __TEST_HELPERS_H__
/*
 * Helper methods for unit tests.
 */

void assert_equals(char a, char b, char* message);

void assert_int_equals(int a, int b, char* message);

void assert_no_error(char* error, char* message);

void assert_error(char* error, char* message);

void assert_true(int boolean, char* message);

void assert_false(int boolean, char* message);

#endif