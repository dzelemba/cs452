#ifndef __ALL_TESTS_H__
#define __ALL_TESTS_H__

/* When you add here add to run_tests.c as well! */

void run_basic_test();
void run_message_passing_test();
void run_multiple_priorities_test();
void run_nameserver_test();
void run_clockserver_test();
void run_task_creation_errors_test();
void run_hwi_test();
void run_uart1_intr_test();
void run_read_sensors_test();

void run_syscall_speed_test();
void run_srr_speed_test();
void run_scheduler_speed_test();

void run_assignment_1_test();
void run_rps_server_test();
void run_assignment_3_test();
void run_assignment_4_test();

void run_train_test();

#endif
