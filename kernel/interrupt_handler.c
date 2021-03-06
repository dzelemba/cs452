#include "debug.h"
#include "events.h"
#include "icu.h"
#include "interrupt_handler.h"
#include "queue.h"
#include "scheduler.h"
#include "task.h"
#include "timer.h"
#include "uart.h"
#include "simple_sm.h"
#include "ts7200.h"

static Task* event_queues[NUM_EVENTS];

// Tracks what events have been turned on.
#define DISABLED 0
#define ENABLED 1
static int event_status[NUM_EVENTS];

// Maps CTS bit + TX bit -> EVENT_SEND_READY
static simple_sm UART1_txReadySM;

void enable_event(int event) {
  switch (event) {
    case EVENT_TIMER:
      enable_interrupt(INTERRUPT_TIMER);
      break;
    case EVENT_UART1_TX_READY:
      ua_enableinterrupts(COM1, TIEN_MASK | MSIEN_MASK);
      enable_interrupt(INTERRUPT_UART1);
      break;
    case EVENT_UART1_RCV_READY:
      ua_enableinterrupts(COM1, RIEN_MASK);
      enable_interrupt(INTERRUPT_UART1);
      break;
    case EVENT_UART2_TX_READY:
      ua_enableinterrupts(COM2, TIEN_MASK);
      enable_interrupt(INTERRUPT_UART2);
      break;
    case EVENT_UART2_RCV_READY:
      ua_enableinterrupts(COM2, RIEN_MASK);
      enable_interrupt(INTERRUPT_UART2);
      break;
    default:
      ERROR("interrupt_handler.c: enable_event: invalid event given: %d\n", event);
  }
  event_status[event] = ENABLED;
}

void init_interrupts() {
  clear_timer_interrupt();
  clear_soft_int();
  reset_interrupts();

  int i;
  for (i = 0; i < NUM_EVENTS; i++) {
    event_queues[i] = (Task *)0;
    event_status[i] = DISABLED;
  }
  sm_create(&UART1_txReadySM, 2);

  set_all_intr_to_irq();

  // We don't get an interrupt for CTS at startup
  if (ua_get_cts_status(COM1)) {
    sm_report_event(&UART1_txReadySM, 0);
  }
}

void reset_interrupts() {
  disable_interrupt(INTERRUPT_TIMER);
  disable_interrupt(INTERRUPT_SOFT);
  disable_interrupt(INTERRUPT_UART1);
  disable_interrupt(INTERRUPT_UART2);

  ua_disableinterrupts(COM1, TIEN_MASK | MSIEN_MASK | RIEN_MASK);
  ua_disableinterrupts(COM2, TIEN_MASK | RIEN_MASK);
}

void send_event(int event, int data) {
  Task* waiting_task = event_queues[event];
  if (waiting_task != (Task *)0) {
    waiting_task->retval = data;
    task_set_state(waiting_task, READY);
    scheduler_add_task(waiting_task->priority, waiting_task);
    event_queues[event] = (Task *)0;
  } else {
    ERROR("Missed Event: %d\n", event);
  }
}

void process_interrupt() {
  if (check_interrupt(INTERRUPT_TIMER)) {
    send_event(EVENT_TIMER, 0);
    clear_timer_interrupt();
  } else if (check_interrupt(INTERRUPT_UART1)) {
    int intStatus = ua_get_intr_status(COM1);
    if (intStatus & MIS_MASK) {
      ASSERT(event_status[EVENT_UART1_TX_READY] == ENABLED, "intr_handler.c: MIS_MASK");
      if (ua_get_cts_status(COM1)) {
        sm_report_event(&UART1_txReadySM, 0);
      }
      ua_clearCTSintr(COM1);
    }
    if (intStatus & TIS_MASK) {
      ASSERT(event_status[EVENT_UART1_TX_READY] == ENABLED, "intr_handler.c: TIS_MASK");
      ASSERT(ua_is_intr_enabled(COM1, TIEN_MASK), "intr_handler.c: TIEN Intr disabled");
      sm_report_event(&UART1_txReadySM, 1);

      // Turn off transmit interrupt
      ua_disableinterrupts(COM1, TIEN_MASK);
    }
    if (intStatus & RIS_MASK) {
      send_event(EVENT_UART1_RCV_READY, ua_getc(COM1));
    }

    if (sm_is_ready(&UART1_txReadySM)) {
      send_event(EVENT_UART1_TX_READY, 0);
      sm_reset(&UART1_txReadySM);
    }
  } else if (check_interrupt(INTERRUPT_UART2)) {
    int intStatus = ua_get_intr_status(COM2);
    if (intStatus & TIS_MASK) {
      ua_disableinterrupts(COM2, TIEN_MASK);
      send_event(EVENT_UART2_TX_READY, 0);
    }
    if (intStatus & RIS_MASK) {
      send_event(EVENT_UART2_RCV_READY, ua_getc(COM2));
    }
  } else if (check_interrupt(INTERRUPT_SOFT)) {
    clear_soft_int();
  } else {
    ERROR("Unknown Interrupt Fired\n");
  }
}

void await_event(Task* task, int event) {
  ASSERT(event_queues[event] == (Task *)0, "await_event called on event with existing listener");

  event_queues[event] = task;

  task_set_state(task, EVENT_BLCK);
  scheduler_remove_task(task->priority);

  if (event_status[event] == DISABLED) {
    enable_event(event);
  }
}
