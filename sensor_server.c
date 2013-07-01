#include "sensor_server.h"
#include "train.h"
#include "stdio.h"
#include "priorities.h"
#include "syscall.h"
#include "debug.h"
#include "sensor.h"
#include "queue.h"
#include "task.h"

/*
 * Private Methods
 */

#define NUM_SENSOR_BYTES (NUM_GROUPS * 2)

#define SENSOR_SERVER_MONITOR_MSG 0
#define SENSOR_SERVER_DATA_MSG    1

static int sensor_server_tid;

typedef struct sensor_server_message {
  int type;
  int num_sensors;
  sensor data[MAX_NEW_SENSORS];
} sensor_server_message;


void prepare_sensors_for_read() {
  putc(COM1, 128 + NUM_GROUPS);
}

void get_sensor_bytes(char* bytes) {
  int i;
  for (i = 0; i < NUM_SENSOR_BYTES; i++) {
    bytes[i] = Getc(COM1);
  }
}

void process_sensor_bytes(char* bytes, sensor_server_message* msg) {
  int i,j;
  int next_sensor = 0;
  for (i = 0; i< NUM_SENSOR_BYTES; i++) {
    int mask = 1;
    int offset = (i % 2) * 8;
    int max = 8 + offset;
    int group = i / 2;
    for (j = max - 1; j >= offset; j--) {
      if (bytes[i] & mask) {
        ASSERT(next_sensor < MAX_NEW_SENSORS, "sensor_server.c : process_sensor_bytes" );
        msg->data[next_sensor++] = (sensor) { group + 'A', j + 1 };
      }
      mask *= 2;
    }
  }
  msg->num_sensors = next_sensor;
}

void sensor_notifier() {
  sensor_server_message msg;
  msg.type = SENSOR_SERVER_DATA_MSG;
  char sensor_bytes[NUM_SENSOR_BYTES];
  while (1) {
    prepare_sensors_for_read();
    get_sensor_bytes(sensor_bytes);
    process_sensor_bytes(sensor_bytes, &msg);

    if (msg.num_sensors != 0) {
      Send(sensor_server_tid, (char *)&msg, sizeof(sensor_server_message), (void *)0, 0);
    }
  }

  Exit();
}

void sensor_server() {
  Create(MED_PRI, &sensor_notifier);

  queue waiting_tasks;
  int q_mem[MAX_TASKS];
  init_queue(&waiting_tasks, q_mem, MAX_TASKS);
  q_set_name(&waiting_tasks, "Sensor Server Waiting Tasks");

  sensor_server_message msg;
  int tid;
  while (1) {
    Receive(&tid, (char *)&msg, sizeof(msg));
    switch (msg.type) {
      case SENSOR_SERVER_MONITOR_MSG:
        push(&waiting_tasks, tid);
        break;
      case SENSOR_SERVER_DATA_MSG:{
        Reply(tid, (void *)0, 0);

        while (!is_queue_empty(&waiting_tasks)) {
          Reply(pop(&waiting_tasks), (char* )msg.data, msg.num_sensors * sizeof(sensor));
        }
        break;
      }
    }
  }
}

/*
 * Public Methods
 */

void start_sensor_server() {
  /* Tell the sensors to reset after dumping data */
  Putc(COM1, 192);

  sensor_server_tid = Create(MED_PRI, &sensor_server);
}

int get_sensor_data(sensor* sensors, int max_sensors) {
  sensor_server_message msg;
  msg.type = SENSOR_SERVER_MONITOR_MSG;
  int bytes_rcvd = Send(sensor_server_tid, (char *)&msg, sizeof(sensor_server_message),
                                           (char *)sensors, max_sensors * sizeof(sensor));

  return bytes_rcvd / sizeof(sensor);
}
