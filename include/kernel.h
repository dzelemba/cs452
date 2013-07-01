#ifndef __KERNEL_H__
#define __KERNEL_H__

void init_kernel();

void kernel_add_task(int priority, void* code);

void kernel_run();


#endif
