#include "../strings.h"
#include "test_helpers.h"
#include <stdio.h>

void string_equal_tests() {
  printf("String Equals Tests\n");

  assert_true(string_equal("Hello", "Hello"), "1");
  assert_true(string_equal("", ""), "2");

  assert_false(string_equal("hello", "Hello"), "3");
  assert_false(string_equal("Helloasdf", "Hello"), "4");
  assert_false(string_equal("Hello", "Helloasdf"), "5");
  assert_false(string_equal("a", ""), "6");
  assert_false(string_equal("", "a"), "7");
}

void test_is_numeric() {
  printf("Is Numeric Tests\n");

  char c;
  for (c = '0'; c <= '9'; c++) {
    assert_true(is_numeric(c), "1");
  }

  assert_false(is_numeric('a'), "2");
}

void test_char_to_int() {
  printf("Char to Int Tests\n");

  char* error = 0;

  char c;
  int i;
  for (c = '0', i = 0; c <= '9'; c++, i++) {
    assert_int_equals(i, char_to_int(c, &error), "1");
    assert_no_error(error, "1");
  }

  char_to_int('a', &error);
  assert_error(error, "2");
}

void test_string_to_int() {
  printf("String to Int Tests\n");

  char* error = 0;

  assert_int_equals(0, string_to_int("0", &error), "1");
  assert_no_error(error, "0");

  assert_int_equals(123, string_to_int("123", &error), "2");
  assert_no_error(error, "2");

  assert_int_equals(2147483647, string_to_int("2147483647", &error), "3");
  assert_no_error(error, "3");

  string_to_int("1b3", &error);
  assert_error(error, "3");

  string_to_int("-13", &error);
  assert_error(error, "4");

  string_to_int("2147483648", &error);
  assert_error(error, "4");

}

void create_string_array_tests() {
  printf("Create String Array Tests\n");

  const char* str_array[3];
  const char static_array[3][2] = {"T1", "T2", "T3"};
  create_string_array((const char*)static_array, 3, 2, (const char**)str_array);
  int i;
  for (i = 0; i < 3; i++) {
    assert_true(string_equal(static_array[i], str_array[i]), "1");
  }
}

int copy_and_check(const char* str, const int size, const char* expected) {
  char output[64];
  string_copy(str, size, output);

  return string_equal(output, expected);
}

void string_copy_tests() {
  printf("String Copy Tests\n");

  assert_true(copy_and_check("Hello", 6, "Hello"), "1");
  assert_true(copy_and_check("Hello", 5, "Hello"), "1");
  assert_true(copy_and_check("Hello", 4, "Hell"), "2");
  assert_true(copy_and_check("Hello", 3, "Hel"), "3");
  assert_true(copy_and_check("Hello", 2, "He"), "4");
  assert_true(copy_and_check("Hello", 1, "H"), "5");
  assert_true(copy_and_check("Hello", 0, ""), "6");
}


int split_and_test(const char* str, const char split, const char** expected_values,
                   const int num_values) {
  char split_values_memory[8][16];
  char* split_values[8];
  create_string_array((const char*)split_values_memory, 8, 16, (const char**)split_values);

  char* error = 0;

  int j,k;
  for (j = 0; j < 8; j++) {
    for (k = 0; k < 16; k++) {
      split_values[j][k] = 'a' + k;
    }
  }

  assert_int_equals(
    num_values,
    string_split(str, split, 8, 16, (char**)split_values, &error),
    "split and test");
  assert_no_error(error, "Split and Test");

  int i;
  for (i = 0; i < num_values; i++) {
    if (!string_equal(split_values[i], expected_values[i])) {
      return 0;
    }
  }

  return 1;
}

void string_split_tests() {
  printf("String Split Tests\n");

  {
    const char expected_memory[3][3] = {"T1", "T2", "T3"};
    const char* expected[3];
    create_string_array((const char*)expected_memory, 3, 3, expected);
    assert_true(split_and_test("T1 T2 T3", ' ', (const char **)expected, 3), "1");
  }

  {
    const char expected_memory[3][8] = {"tr", "5", "43"};
    const char* expected[3];
    create_string_array((const char*)expected_memory, 3, 8, expected);
    assert_true(split_and_test("tr 5 43", ' ', (const char **)expected, 3), "2");
  }

  {
    const char expected_memory[3][8] = {"tr", "5", "43"};
    const char* expected[3];
    create_string_array((const char*)expected_memory, 3, 8, expected);
    assert_true(split_and_test("  tr   5  43   ", ' ', (const char **)expected, 3), "3");
  }
}

void strings_tests() {
  printf("******* Strings Tests ********\n\n");

  string_equal_tests();
  test_is_numeric();
  test_char_to_int();
  test_string_to_int();
  create_string_array_tests();
  string_copy_tests();
  string_split_tests();

  printf("\n");
}
