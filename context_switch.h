#ifndef __CONTEXT_SWITCH_H__ 
#define __CONTEXT_SWITCH_H__ 

void k_enter();

void k_exit(char* user_stack, void (*user_spsr));

#endif 
