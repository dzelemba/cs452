#ifndef __STRINGS_H__
#define __STRINGS_H__

/*
 * String utility functions.
 */

/*
 * Splits a string based on a given character.
 *
 * Parameters:
 *  - str: string to split. Must be null-terminated.
 *  - split: character to split on
 *  - output_size: size of the output array
 *  - string_size: size of each individual string.
 *  - output: array of strings. MUST be created using "create_string_array".
 *  - error: error message if an error occurred
 *
 * Return Value: number of tokens found.
 */
int string_split(const char* str, const char split, const int output_size,
                 const int string_size, char** output, char** error);

/*
 * Copies a string.
 *
 * Parameters:
 *  - input: string to copy.
 *  - max_size: max number of bytes to copy.
 *  - output: location of new string. Must of size at least "size + 1".
 */
void string_copy(const char* input, const int max_size, char* output);

/*
 * Checks whether two strings are equal.
 * Both strings must be null-terminated.
 */
int string_equal(const char* str1, const char* str2);

/*
 * Creates a normal string array. C turns char str[4][5] in one big array
 * of chars, so this converts it into a real array of strings
 *
 * Parameters:
 *  - array_of_strings: statically created array of strings, i.e. char str[3][4].
 *  - num_strings: number of strings in array.
 *  - string_size: size of each string in the arryay.
 *  - output: array of char* to put the strings into.
 */
void create_string_array(const char* array_of_strings, const int num_strings,
                         const int string_size, const char** output);

/*
 * Returns true if the char is bewtwee '0' and '9'.
 */
int is_numeric(char c);

/*
 * Converts a char to an int
 */
int char_to_int(char c, char** error);

/*
 * Converts a string to an int.
 * Doesn't support negative numbers.
 */
int string_to_int(const char* str, char** error);

#endif
