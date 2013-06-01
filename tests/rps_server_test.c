#include "all_tests.h"
#include "kernel.h"
#include <bwio.h>
#include "syscall.h"
#include "test_helpers.h"
#include "rps_server.h"
#include "priorities.h"

#define NUM_ROUNDS 5

static void player_play(int player, int* moves, int num_moves) {
  bwprintf(COM2, "Player %d Starting...\n", player);

  int rps_server = get_rps_tid();
  bwprintf(COM2, "Player %d Acquired RPS Tid...\n", player);

  int retval = signup(rps_server);
  assert_int_equals(OK, retval, "RPS Test: Player Signup");
  bwprintf(COM2, "Player %d Signed Up...\n", player);

  int i;
  for (i = 0; i < num_moves; i++) {
    int move = moves[i];
    play(rps_server, move);
  }

  retval = quit(rps_server);
  assert_int_equals(SUCCESSFUL_QUIT, retval, "RPS Test: Player Quit");

  bwprintf(COM2, "Player %d Done...\n", player);
}

static void player_1() {
  int moves[NUM_ROUNDS] = {ROCK, PAPER, ROCK, SCISSORS, PAPER};
  player_play(MyTid(), moves, NUM_ROUNDS);

  // Play again.
  player_play(MyTid(), moves, NUM_ROUNDS);

  Exit();
}

static void player_2() {
  int moves[NUM_ROUNDS] = {ROCK, SCISSORS, SCISSORS, PAPER, ROCK};
  player_play(MyTid(), moves, NUM_ROUNDS);

  Exit();
}

static void player_3() {
  int moves[NUM_ROUNDS] = {PAPER, SCISSORS, ROCK, PAPER, SCISSORS};
  player_play(MyTid(), moves, NUM_ROUNDS);

  // Play again.
  player_play(MyTid(), moves, NUM_ROUNDS);

  Exit();
}

static void player_4() {
  int moves[NUM_ROUNDS] = {SCISSORS, SCISSORS, PAPER, SCISSORS, ROCK};
  player_play(MyTid(), moves, NUM_ROUNDS);

  Exit();
}

static void first() {
  bwprintf(COM2, "Rock Paper Scissor Server Starting...\n\n");

  start_rps_server();

  Create(LOW_PRI, &player_1);
  Create(LOW_PRI, &player_2);
  Create(LOW_PRI, &player_3);
  Create(LOW_PRI, &player_4);

  Exit();
}

void run_rps_server_test() {
  init_kernel();
  reset_did_fail();

  kernel_add_task(MED_PRI, &first);

  kernel_run();
}
