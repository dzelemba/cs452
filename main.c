#include "syscall.h"
#include <bwio.h>
#include "task.h"
#include "context_switch.h"

void get_status(int status) {
  asm("ldr r3, [R14, #-4]");
  asm("bic r3,r3, #0xFF000000");
}

void user_task() {
  bwprintf(COM2, "User task! 1\n");

  Pass();

  bwprintf(COM2, "User task! 2\n");

  Pass();

  bwprintf(COM2, "User task! 3\n");

  Pass();
  bwprintf(COM2, "User task! 4\n");

  Pass();
  bwprintf(COM2, "User task! 5\n");

  Exit();
}

void process_request(Task* task, Request* request) {
  int retval = 0;

  Task* new_task;
  switch (request->syscall) {
    case 0:
      new_task = task_create(task->tid, (void *)request->args[1]);
      if (new_task) {  
        // scheduler_add_task(request->args[0], new_task) 
      } else {
        retval = -2; // Kernel out of task descriptors
      }
      break;
    case 1:
      retval = task->tid;
      break;
    case 2:
      retval = task->parent_tid;
      break;
    case 3:
      // scheduler_move_to_back(task); 
      break;
    case 4:
      // scheduler_remove_task(task)
      break;
    default:
      bwprintf(COM2, "Illegal syscall number: %d\n", request->syscall);
  }

  // TODO: Insert retval into user stack.
}

int main(int argc, char** argv) {
  // First set up the kernel. 
  *(int *)(0x28) = (int)&k_enter; 
  init_tasks();

  Task* first_task = task_create(-1 /* Parent tid */, &user_task);
  
  Task* next_task;
  //while (1) {
  int i =0;
  for (; i < 5; i++) {
    next_task = first_task; //scheduler_get_next_task();
    Request* request = k_exit(&next_task->stack_position);
    process_request(next_task, request); 
  }

  bwprintf(COM2, "Main Exiting... \n");

  return 0;
}
