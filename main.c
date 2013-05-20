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

  int tid = MyTid();
  bwprintf(COM2, "User task tid: %d\n", tid);

  int p_tid = MyParentTid();
  bwprintf(COM2, "User task parent tid: %d\n", p_tid);

  Pass();
  bwprintf(COM2, "User task! 5\n");

  Exit();
}

int process_request(Task* task, Request* request) {
  Task* new_task;
  switch (request->syscall) {
    case 0:
      new_task = task_create(task->tid, (void *)request->args[1]);
      if (new_task) {  
        // scheduler_add_task(request->args[0] /* Priority */, new_task) 
      } else {
        return -2; // Kernel out of task descriptors
      }
      break;
    case 1:
      return task->tid;
    case 2:
      return task->parent_tid;
    case 3:
      // scheduler_move_to_back(priority);
      break;
    case 4:
      // scheduler_remove_task(priority) Just pop the first.
      break;
    default:
      bwprintf(COM2, "Illegal syscall number: %d\n", request->syscall);
  }

  return 0;
}

int main(int argc, char** argv) {
  // First set up the kernel. 
  *(int *)(0x28) = (int)&k_enter; 
  init_tasks();

  Task* first_task = task_create(-1 /* Parent tid */, &user_task);
  
  Task* next_task;
  int user_retval = 0;
  //while (1) {
  int i =0;
  for (; i < 5; i++) {
    next_task = first_task; //scheduler_get_next_task();
    Request* request = k_exit(user_retval, &next_task->stack_position);
    user_retval = process_request(next_task, request); 
  }

  bwprintf(COM2, "Main Exiting... \n");

  return 0;
}
