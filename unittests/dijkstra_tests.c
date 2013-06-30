#include "../dijkstra.h"
#include "../track_data.h"
#include "../track_node.h"

#include <stdio.h>

void dijkstra_tests() {
  printf("******* Dijkstra Tests ********\n\n");

  sequence path[TRACK_MAX];
  int pathlen;

  track_node* track = get_track();
  init_tracka(track);

  get_path_debug(track, 'C', 9, 'A', 12, path, &pathlen);

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
}
