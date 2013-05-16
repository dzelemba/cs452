#include "syscall.h"
#include <bwio.h>
#include "task.h"

void get_status(int status) {
  asm("ldr r3, [R14, #-4]");
  asm("bic r3,r3, #0xFF000000");
}

void exception_handler() {
  asm("mov r1, r0");
  asm("mov r0, #1");
  asm("bl bwputr");

  bwprintf(COM2, "Yo yo\n");
}

void user_task() {
  bwprintf(COM2, "User task!\n");
}

void start_task(struct task* t) {
  
}

int main(int argc, char** argv) {
  // First set up the kernel. 
  *(int *)(0x28) = (int)&exception_handler; 

  struct task first_task; 
  first_task.tid = 1;
  first_task.code = &user_task;
  
  start_task(&first_task);

  bwprintf(COM2, "Yo yo2\n");
 
  return 0;
}
