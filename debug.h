#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG_1
  #define ASSERT(exp, description) do { assert((exp), #exp, description); } while(0)
#else
  #define ASSERT(exp, description) do { } while (0)
#endif

#ifdef DEBUG
  #define PRINT_DEBUG(...) do { print_debug(__VA_ARGS__); } while (0)
  #define METHOD_ENTRY(...) do { method_entry(__VA_ARGS__); } while (0)
  #define METHOD_EXIT(...) do { method_exit(__VA_ARGS__); } while (0)
#else
  #define PRINT_DEBUG(...) do { } while (0)
  #define METHOD_ENTRY(...) do { } while (0)
  #define METHOD_EXIT(...) do { } while (0)
#endif

void print_debug(char* format, ...);
void method_entry(char* format, ...);
void method_exit(char* format, ...);
void assert(int exp, char* exp_str, char* msg);

#endif
