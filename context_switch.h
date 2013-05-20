#ifndef __CONTEXT_SWITCH_H__ 
#define __CONTEXT_SWITCH_H__ 

void k_enter();

Request* k_exit(int retval, int** user_stack);

#endif 
