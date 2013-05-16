#include "syscall.h"
#include <bwio.h>
#include "task.h"
#include "exception_handler.h"

void get_status(int status) {
  asm("ldr r3, [R14, #-4]");
  asm("bic r3,r3, #0xFF000000");
}

/* Printing Registers
  asm("mov r1, r0");
  asm("mov r0, #1");
  asm("bl bwputr");
*/
void user_task() {
  bwprintf(COM2, "User task!\n");

  Exit();
}

/* 
 * Be careful changing any code here, GCC might use the stack ptr and some instructions
 * would need to be modified.
 */
void start_task(char* stack, void (*code)) {
  // GCC doesn't move the stack ptr after it saves a bunch of stuff to it, so we have to do it.
  asm("sub sp, sp, #8"); 
  
  // Save kernel's registers.
  asm("stmdb sp, {r0, r1, r2, r3, r4, r5, r6, r7, r8, sb, sl, fp, ip, lr}");

/* Debug Print
  asm("mov r6, r1");
  asm("mov r7, r0");

  asm("mov r1, sp");
  asm("mov r0, #1");
  asm("bl bwputr");

  asm("mov r1, r6");
  asm("mov r0, r7");
*/

  // Switch to system mode.
  asm("mrs r2, cpsr");
  asm("bic r2, r2, #0x1f");
  asm("orr r2, r2, #0x1f");
  asm("msr cpsr_c, r2");

  // Restore task's registers (just stack ptr for now).
  asm("mov sp, r0");

  // Back to supervisor mode.
  asm("mrs r2, cpsr");
  asm("bic r2, r2, #0x1f");
  asm("orr r2, r2, #0x13");
  asm("msr cpsr_c, r2");

  // Jump to user task.
  asm("mov pc, r1");
}

int main(int argc, char** argv) {
  // First set up the kernel. 
  *(int *)(0x28) = (int)&handle_exception; 

  struct task first_task; 
  first_task.tid = 1;
  first_task.code = &user_task;

  bwprintf(COM2, "User StacK Base: %x\n", first_task.stack + STACK_SIZE);
  bwprintf(COM2, "Some Point on Kernel StacK: ");
  asm("mov r1, sp");
  asm("mov r0, #1");
  asm("bl bwputr");
  bwprintf(COM2, "\n");
  
  start_task(first_task.stack + STACK_SIZE, first_task.code);

  bwprintf(COM2, "Main Exiting... \n");
 
  return 0;
}
