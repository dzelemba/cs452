#include "syscall.h"
#include <bwio.h>
#include "task.h"
#include "context_switch.h"

void get_status(int status) {
  asm("ldr r3, [R14, #-4]");
  asm("bic r3,r3, #0xFF000000");
}

void user_task() {
  bwprintf(COM2, "User task!\n");

  Exit();
}

int main(int argc, char** argv) {
  // First set up the kernel. 
  *(int *)(0x28) = (int)&k_enter; 

  struct task first_task; 
  first_task.tid = 1;
  first_task.code = &user_task;
  
  k_exit(first_task.stack + STACK_SIZE, first_task.code);

  bwprintf(COM2, "Main Exiting... \n");
 
  return 0;
}
