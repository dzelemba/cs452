#ifndef __RESERVATION_SERVER_H__
#define __RESERVATION_SERVER_H__

#include "ourlib.h"
#include "track_edge_array.h"
#include "track_node.h"
#include "train.h"

typedef enum rs_reply {
  SUCCESS,
  FAIL
} rs_reply;

typedef enum edge_status {
  FREE,
  RESERVED
} edge_status;

typedef struct get_all_updates_reply {
  track_edge* edge;
  edge_status status;
  int train;
} get_all_updates_reply;

void init_reservation_server();

rs_reply rs_reserve(int train, track_edge* edge);

void rs_free(int train, track_edge* edge);

void rs_get_updates(train_array* tr_array);

void rs_get_all_updates(get_all_updates_reply *reply);

/*
 * Helpers for reserving through track_edge_arrays
 */

bool is_edge_free(track_edge* edge, track_edge_array* edge_statuses);

void reserve_edge(track_edge* edge, track_edge_array* edge_statuses);

void free_edge(track_edge* edge, track_edge_array* edge_statuses);

void store_value_at_edge(track_edge* edge, track_edge_array* edge_statuses, int val);

int get_value_at_edge(track_edge* edge, track_edge_array* edge_statuses);

#endif
