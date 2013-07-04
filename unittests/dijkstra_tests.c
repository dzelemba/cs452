#include "dijkstra.h"
#include "track_data.h"
#include "track_node.h"

#include <stdio.h>

void dijkstra_tests() {
  printf("******* Dijkstra Tests ********\n\n");

  sequence path[TRACK_MAX];
  int pathlen;

  track_node* track = get_track();
  init_tracka(track);

  get_path_from_idx(track, sensor2idx('B', 15), branch2idx(13), path, &pathlen);

  int i;
  for (i = 0; i < pathlen; i++) {
    printf("%s - ", track[path[i].location].name);
    if (path[i].action == DO_NOTHING) {
      printf("do nothing\n");
    } else if (path[i].action == REVERSE) {
      printf("reverse\n");
    } else if (path[i].action == TAKE_STRAIGHT) {
      printf("take straight\n");
    } else if (path[i].action == TAKE_CURVE) {
      printf("take curve\n");
    }
  }

  printf("\n");

  printf("******* Needed Calibration Distances ********\n\n");
  printf("******* TRACK A ********\n\n");

  // Inner loop
  printf("A4 -> B16: %d\n", get_path_debug(track, 'A', 4, 'B', 16, path, &pathlen));
  printf("B16 -> C10: %d\n", get_path_debug(track, 'B', 16, 'C', 10, path, &pathlen));
  printf("C10 -> B1: %d\n", get_path_debug(track, 'C', 10, 'B', 1, path, &pathlen));
  printf("B1 -> D14: %d\n", get_path_debug(track, 'B', 1, 'D', 14, path, &pathlen));
  printf("D14 -> E14: %d\n", get_path_debug(track, 'D', 14, 'E', 14, path, &pathlen));
  printf("E14 -> E9: %d\n", get_path_debug(track, 'E', 14, 'E', 9, path, &pathlen));
  printf("E9 -> D5: %d\n", get_path_debug(track, 'E', 9, 'D', 5, path, &pathlen));
  printf("D5 -> E6: %d\n", get_path_debug(track, 'D', 5, 'E', 6, path, &pathlen));
  printf("E6 -> D4: %d\n", get_path_debug(track, 'E', 6, 'D', 4, path, &pathlen));
  printf("D4 -> B6: %d\n", get_path_debug(track, 'D', 4, 'B', 6, path, &pathlen));
  printf("B6 -> C12: %d\n", get_path_debug(track, 'B', 6, 'C', 12, path, &pathlen));
  printf("C12 -> A4: %d\n", get_path_debug(track, 'C', 12, 'A', 4, path, &pathlen));

  /*printf("B3 -> C2: %d\n", get_path_debug(track, 'B', 3, 'C', 2, path, &pathlen));*/
  /*printf("C15 -> D12: %d\n", get_path_debug(track, 'B', 3, 'C', 2, path, &pathlen));*/

  printf("\n");
}
