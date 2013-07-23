#include "reservation_server.h"
#include "syscall.h"
#include "track_data.h"
#include "queue.h"
#include "debug.h"
#include "priorities.h"
#include "track_edge_array.h"

/*
 * Reservation Server
 */

static int reservation_server_tid;

typedef enum reservation_server_message_type {
  RS_RESERVE,
  RS_FREE,
  RS_GET_FREE_UPDATES,
  RS_GET_ALL_UPDATES
} reservation_server_message_type;

typedef struct reservation_server_message {
  reservation_server_message_type type;
  int train;
  track_edge* edge;
} reservation_server_message;

void reply_to_all_updates(int* tid, track_edge* edge, edge_status status, int train) {
  if (*tid != 0) {
    get_all_updates_reply reply;
    reply.edge = edge;
    reply.status = status;
    reply.train = train;
    Reply(*tid, (char *)&reply, sizeof(get_all_updates_reply));
    *tid = 0;
  } else {
    ERROR("No one waiting for all updates");
  }
}

void reservation_server() {
  track_edge_array edge_statuses;
  tea_set_name(&edge_statuses, "Reservation Server Array");
  clear_track_edge_array(&edge_statuses);

  // Trains that have failed to reserve a node.
  queue blocked_trains;
  Q_CREATE(blocked_trains, MAX_TRAINS);
  q_set_name(&blocked_trains, "reservation_server queue");

  // Task id of the task waiting to hear about trains
  // that were waiting on nodes that have been freed up.
  int waiting_tid_free_updates = 0;
  int* blocked_trains_mem;
  int num_blocked_trains;

  // Task id of task waiting to hear about any reservation change.
  int waiting_tid_all_updates = 0;

  rs_reply reply;
  int tid;
  reservation_server_message msg;
  while (1) {
    Receive(&tid, (char *)&msg, sizeof(reservation_server_message));
    switch (msg.type) {
      case RS_RESERVE:
        if (is_edge_free(msg.edge, &edge_statuses)) {
          INFO(RESERVATION_SERVER, "Train %d Reserved %s -> %s",
               msg.train, msg.edge->src->name, msg.edge->dest->name);
          reserve_edge(msg.edge, &edge_statuses);
          reply = SUCCESS;
          reply_to_all_updates(&waiting_tid_all_updates, msg.edge, RESERVED, msg.train);
        } else {
          INFO(RESERVATION_SERVER, "Train %d Blocked on %s -> %s",
               msg.train, msg.edge->src->name, msg.edge->dest->name);
          push(&blocked_trains, msg.train);
          reply = FAIL;
        }
        Reply(tid, (char *)&reply, sizeof(rs_reply));
        break;
      case RS_FREE:
        Reply(tid, (char *)0, 0);
        ASSERT(!is_edge_free(msg.edge, &edge_statuses),
               "Reservation Server: train %d freeing %s -> %s",
               msg.train, msg.edge->src->name, msg.edge->dest->name);

        free_edge(msg.edge, &edge_statuses);
        INFO(RESERVATION_SERVER, "Train %d Freed %s -> %s",
             msg.train, msg.edge->src->name, msg.edge->dest->name);
        reply_to_all_updates(&waiting_tid_all_updates, msg.edge, FREE, msg.train);

        if (queue_size(&blocked_trains) != 0) {
          if (waiting_tid_free_updates == 0) {
            INFO(RESERVATION_SERVER, "No one waiting for freed nodes");
          } else {
            queue_get_mem(&blocked_trains, &blocked_trains_mem, &num_blocked_trains);
            Reply(waiting_tid_free_updates, (char *)blocked_trains_mem, num_blocked_trains * sizeof(int));
            queue_clear(&blocked_trains);
            waiting_tid_free_updates = 0;
          }
        }
        break;
      case RS_GET_FREE_UPDATES:
        waiting_tid_free_updates = tid;
        break;
      case RS_GET_ALL_UPDATES:
        waiting_tid_all_updates = tid;
        break;
    }
  }
}

/*
 * Public Methods
 */

void init_reservation_server() {
  reservation_server_tid = Create(MED_PRI_1, &reservation_server);
}

rs_reply rs_reserve(int train, track_edge* edge) {
  reservation_server_message msg;
  msg.type = RS_RESERVE;
  msg.train = train;
  msg.edge = edge;

  rs_reply reply;
  Send(reservation_server_tid, (char *)&msg, sizeof(reservation_server_message),
                               (char *)&reply, sizeof(rs_reply));
  return reply;
}

void rs_free(int train, track_edge* edge) {
  reservation_server_message msg;
  msg.type = RS_FREE;
  msg.train = train;
  msg.edge = edge;
  Send(reservation_server_tid, (char *)&msg, sizeof(reservation_server_message), (char *)0, 0);
}

void rs_get_updates(train_array* tr_array) {
  reservation_server_message msg;
  msg.type = RS_GET_FREE_UPDATES;

  int size = Send(reservation_server_tid, (char *)&msg, sizeof(reservation_server_message),
                                          (char *)tr_array->trains, sizeof(train_array));
  tr_array->size = size / sizeof(int);
}

void rs_get_all_updates(get_all_updates_reply *reply) {
  reservation_server_message msg;
  msg.type = RS_GET_ALL_UPDATES;

  Send(reservation_server_tid, (char *)&msg, sizeof(reservation_server_message),
                               (char *)reply, sizeof(get_all_updates_reply));
}

/*
 * Helpers for reserving through track_edge_arrays
 */

bool is_edge_free(track_edge* edge, track_edge_array* edge_statuses) {
  track_edge* edge_group[MAX_EDGE_GROUP_SIZE];
  int num_edges = get_edge_group(edge, edge_group);

  int i = 0;
  for (i = 0; i < num_edges; i++) {
    if (isset_edge(edge_statuses, edge_group[i])) {
      return false;
    }
  }

  return true;
}

void reserve_edge(track_edge* edge, track_edge_array* edge_statuses) {
  ASSERT(is_edge_free(edge, edge_statuses), "reservation_server: reserve_edge: %s -> %s in %s",
         edge->src->name, edge->dest->name, edge_statuses->name);
  track_edge* edge_group[MAX_EDGE_GROUP_SIZE];
  int num_edges = get_edge_group(edge, edge_group);

  int i = 0;
  for (i = 0; i < num_edges; i++) {
    set_edge(edge_statuses, edge_group[i]);
  }
}

void free_edge(track_edge* edge, track_edge_array* edge_statuses) {
  ASSERT(!is_edge_free(edge, edge_statuses), "reservation_server: free_edge: %s -> %s in %s",
         edge->src->name, edge->dest->name, edge_statuses->name);
  track_edge* edge_group[MAX_EDGE_GROUP_SIZE];
  int num_edges = get_edge_group(edge, edge_group);

  int i = 0;
  for (i = 0; i < num_edges; i++) {
    unset_edge(edge_statuses, edge_group[i]);
  }
}

void store_value_at_edge(track_edge* edge, track_edge_array* edge_statuses, int val) {
  ASSERT(is_edge_free(edge, edge_statuses), "reservation_server: store_at_edge: %s -> %s in %s",
         edge->src->name, edge->dest->name, edge_statuses->name);
  track_edge* edge_group[MAX_EDGE_GROUP_SIZE];
  int num_edges = get_edge_group(edge, edge_group);

  int i = 0;
  for (i = 0; i < num_edges; i++) {
    set_edge_value(edge_statuses, edge_group[i], val);
  }
}

int get_value_at_edge(track_edge* edge, track_edge_array* edge_statuses) {
  track_edge* edge_group[MAX_EDGE_GROUP_SIZE];
  int num_edges = get_edge_group(edge, edge_group);

  return get_edge_value(edge_statuses, edge_group[0]);
}
