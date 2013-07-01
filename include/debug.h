#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG
  #define DEBUG_3 1
  #define PRINT_DEBUG(...) do { print_debug(__VA_ARGS__); } while (0)
  #define METHOD_ENTRY(...) do { method_entry(__VA_ARGS__); } while (0)
  #define METHOD_EXIT(...) do { method_exit(__VA_ARGS__); } while (0)
#else
  #define PRINT_DEBUG(...) do { } while (0)
  #define METHOD_ENTRY(...) do { } while (0)
  #define METHOD_EXIT(...) do { } while (0)
#endif

#ifdef DEBUG_3
  #define DEBUG_2 1
  #define INFO(...) do { print_debug("INFO: "); print_debug(__VA_ARGS__); } while (0)
  #define USER_INFO(...) do { print_debug("USER INFO: "); print_debug(__VA_ARGS__); } while (0)
#else
  #define INFO(...) do { } while (0)
  #define USER_INFO(...) do { } while (0)
#endif

#ifdef DEBUG_2
  #define DEBUG_1 1
  #define WARNING(...) do { print_debug("WARNING: "); print_debug(__VA_ARGS__); } while (0)
#else
  #define WARNING(...) do { } while (0)
#endif

#ifdef DEBUG_1
  #define ASSERT(exp, description) do { assert((exp), #exp, description); } while(0)
  #define ERROR(...) do { error(__VA_ARGS__); } while (0)
#else
  #define ASSERT(exp, description) do { } while (0)
  #define ERROR(...) do { } while (0)
#endif

void print_debug(char* format, ...);
void method_entry(char* format, ...);
void method_exit(char* format, ...);
void assert(int exp, char* exp_str, char* msg);
void error(char* format, ...);

#endif
