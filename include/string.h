#ifndef __STRING_H__
#define __STRING_H__

typedef struct string {
  char* chars;
  int max_size;
  int size;
} string;

#define STR_CREATE(s, size) char mem[size]; str_create(&s, mem, size); str_create(&s, mem, size);

void str_create(string* s, char* mem, int size);

char* str_get_chars(string* s);

int str_get_size(string* s);

void str_append(string* s, char c);

#endif
