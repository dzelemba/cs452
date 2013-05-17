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
  task_create(&first_task, 1, &user_task);
  
  k_exit(task_get_stack(&first_task));

  bwprintf(COM2, "Main Exiting... \n");
 
  return 0;
}
