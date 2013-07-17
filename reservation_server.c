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
  RS_GET_UPDATES
} reservation_server_message_type;

typedef struct reservation_server_message {
  reservation_server_message_type type;
  int train;
  track_edge* edge;
} reservation_server_message;

#define MAX_EDGE_GROUP_SIZE 4

void get_edge_group(track_edge* edge, track_edge** edge_group, int* size) {
  int i = 0;
  edge_group[i++] = edge;
  edge_group[i++] = edge->reverse;

  track_edge* edges = 0;
  if (edge->src->type == NODE_BRANCH) {
    edges = edge->src->edge;
  } else if (edge->dest->type == NODE_MERGE) {
    edges = edge->dest->reverse->edge;
    edge = edge->reverse;
  }
  if (edges != 0) {
    track_edge* other_edge = (&edges[0] == edge ? &edges[1] : &edges[0]);
    edge_group[i++] = other_edge;
    edge_group[i++] = other_edge->reverse;
  }

  *size = i;
}

int is_edge_free(track_edge* edge, track_edge_array* edge_statuses) {
  track_edge* edge_group[MAX_EDGE_GROUP_SIZE];
  int num_edges;
  get_edge_group(edge, edge_group, &num_edges);

  int i = 0;
  for (i = 0; i < num_edges; i++) {
    if (isset_edge(edge_statuses, edge_group[i])) {
      return 0;
    }
  }

  return 1;
}

void reserve_edge(track_edge* edge, track_edge_array* edge_statuses) {
  ASSERT(is_edge_free(edge, edge_statuses), "reservation_server: reserve_edge");
  track_edge* edge_group[MAX_EDGE_GROUP_SIZE];
  int num_edges;
  get_edge_group(edge, edge_group, &num_edges);

  int i = 0;
  for (i = 0; i < num_edges; i++) {
    set_edge(edge_statuses, edge_group[i]);
  }
}

void free_edge(track_edge* edge, track_edge_array* edge_statuses) {
  ASSERT(!is_edge_free(edge, edge_statuses), "reservation_server: reserve_edge");
  track_edge* edge_group[MAX_EDGE_GROUP_SIZE];
  int num_edges;
  get_edge_group(edge, edge_group, &num_edges);

  int i = 0;
  for (i = 0; i < num_edges; i++) {
    unset_edge(edge_statuses, edge_group[i]);
  }
}

void reservation_server() {
  track_edge_array edge_statuses;
  clear_track_edge_array(&edge_statuses);

  // Trains that have failed to reserve a node.
  queue blocked_trains;
  Q_CREATE(blocked_trains, MAX_TRAINS);

  // Task id of the task waiting to hear about trains
  // that were waiting on nodes that have been freed up.
  int waiting_tid = 0;
  int* blocked_trains_mem;
  int num_blocked_trains;

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

        if (queue_size(&blocked_trains) != 0) {
          if (waiting_tid == 0) {
            INFO(RESERVATION_SERVER, "No one waiting for freed nodes");
          } else {
            queue_get_mem(&blocked_trains, &blocked_trains_mem, &num_blocked_trains);
            Reply(waiting_tid, (char *)blocked_trains_mem, num_blocked_trains * sizeof(int));
            queue_clear(&blocked_trains);
            waiting_tid = 0;
          }
        }
        break;
      case RS_GET_UPDATES:
        waiting_tid = tid;
        break;
    }
  }
}

/*
 * Public Methods
 */

void init_reservation_server() {
  reservation_server_tid = Create(MED_PRI, &reservation_server);
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
  msg.type = RS_GET_UPDATES;

  int size = Send(reservation_server_tid, (char *)&msg, sizeof(reservation_server_message),
                                          (char *)tr_array->trains, sizeof(train_array));
  tr_array->size = size / sizeof(int);
}

