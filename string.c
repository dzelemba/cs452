#include "string.h"
#include "debug.h"

void str_create(string* s, char* mem, int size) {
  s->chars = mem;
  s->max_size = size;
  s->size = 0;

  int i;
  for (i = 0; i < size; i++) {
    mem[i] = '\0';
  }
}

char* str_get_chars(string* s) {
  return s->chars;
}

int str_get_size(string* s) {
  return s->size;
}

void str_append(string* s, char c) {
  ASSERT(s->size < s->max_size, "string.c: str_append: string full");

  s->chars[s->size] = c;
  s->size++;
}
