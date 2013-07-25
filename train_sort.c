#include "train_sort.h"
#include "train.h"
#include "track_data.h"
#include "location.h"

void send_train(int train, location* dest) {
  if (train != 0) {
    tr_set_route(train, DEFAULT_TRAIN_SPEED, dest);
  }
}

void sort_trains(int t1, int t2, int t3, int t4) {
  location dest;
  init_location(&dest);

  dest.node = get_track_node(sensor2idx('C', 15));
  send_train(t1, &dest);

  dest.node = get_track_node(sensor2idx('D', 12));
  send_train(t2, &dest);

  dest.node = get_track_node(sensor2idx('E', 11));
  send_train(t3, &dest);

  dest.node = get_track_node(sensor2idx('D', 12));
  send_train(t4, &dest);
}
