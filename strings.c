#include "strings.h"

void perform_split(const char* str, const int output_size,
                  const int string_size, char** output, char** error,
                  const int i, const int last_split, const int output_index) {
  // First check errors.
  int size = i - last_split;
  if (size > string_size) {
    *error = "string_size not large enough";
  }

  if (output_index > output_size - 1) {
    *error = "output_size not large enough";
  }

  // Otherwise we're good!
  string_copy(&str[last_split], size, output[output_index]);
}

int string_split(const char* str, const char split, const int output_size,
                 const int string_size, char** output, char** error) {
  int i;
  int last_split = 0;
  int output_index = 0;
  for (i = 0; str[i] != '\0'; i++) {
    if (str[i] == split) {
      if (i != last_split) {
        perform_split(str, output_size, string_size, output,
                      error, i, last_split, output_index);
        if (*error) return -1;

        output_index++;
        last_split = i + 1;
      } else {
        last_split++;
      }
    }
  }
  // Get last token.
  if (i != last_split) {
    perform_split(str, output_size, string_size, output,
                  error, i, last_split, output_index);
    if (*error) return -1;

    output_index++;
  }

  return output_index;
}

void string_copy(const char* input, const int max_size, char* output) {
  int i;
  for (i = 0; input[i] != '\0' && i < max_size; i++) {
    output[i] = input[i];
  }
  output[i] = '\0';
}


int string_equal(const char* str1, const char* str2) {
  int i;
  for (i = 0; str1[i] != '\0' || str2[i] != '\0'; i++) {
    if (str1[i] != str2[i]) {
      return 0;
    }
  }

  return 1;
}

void create_string_array(const char* array_of_strings, const int num_strings,
                         const int string_size, const char** output) {
  int i;
  for (i = 0; i < num_strings; i++) {
    output[i] = array_of_strings + i * string_size;
  }
}

int is_numeric(char c) {
  return c >= '0' && c <= '9';
}

int char_to_int(char c, char** error) {
  if (!is_numeric(c)) {
    *error = "Invalid integer";
    return -1;
  }

  return c - '0';
}

int string_to_int(const char* str, char** error) {
  int result = 0;

  // Find end of array.
  int num_digits;
  for (num_digits = 0; str[num_digits] != '\0'; num_digits++);

  // Now parse.
  int i;
  int multiplier = 1;
  for (i = num_digits - 1; i >= 0; i--) {
    int c = char_to_int(str[i], error);
    if (*error) return -1;

    result += multiplier * c;
    multiplier *= 10;

    // Check for overflow.
    if (result < 0) {
      *error = "Value too big for integer";
      return -1;
    }
  }

  return result;
}
