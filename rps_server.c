#include "rps_server.h"
#include "task.h"
#include "bwio.h"
#include "syscall.h"
#include "priorities.h"

/* Private Methods */

// Request Types
#define SIGNUP 0
#define PLAY 1
#define QUIT 2

typedef struct rps_request {
  int type;
  int move;
} rps_request;

static int waiting_to_play;

// Index i in players is who tid i is playing against.
// -1 represents no one.
static int players[MAX_TASKS + 1];

// Index i represents the move tid i played.
// -1 represents no move yet.
// -2 represents the player quit.
static int moves[MAX_TASKS + 1];

static int move_outcomes[3][3];

static char* move_to_string(int result) {
  switch (result) {
    case ROCK:
      return "ROCK";
    case PAPER:
      return "PAPER";
    case SCISSORS:
      return "SCISSORS";
    default:
      return "UNKNOWN MOVE";
  }
}

static char* result_to_string(int result) {
  switch (result) {
    case WIN:
      return "WIN";
    case LOSE:
      return "LOST";
    case TIE:
      return "TIE";
    case PLAYER_QUIT:
      return "PLAYER QUIT";
    case NO_GAME_IN_PROGRESS:
      return "NO GAME IN PROGRESS";
    case INVALID_MOVE:
      return "INVALID MOVE";
    default:
      return "UNKNOWN RESULT";
  }
}

void reply(int tid, int reply) {
  Reply(tid, (char *)&reply, sizeof(int));
}

int is_valid_move(int move) {
  return move >= ROCK && move <= SCISSORS;
}

void _signup(int tid) {
  if (players[tid] != -1) {
    reply(tid, ALREADY_PLAYING);
  } else if (waiting_to_play == -1) {
    waiting_to_play = tid;
  } else {
    players[tid] = waiting_to_play;
    players[waiting_to_play] = tid;

    reply(tid, OK);
    reply(waiting_to_play, OK);

    waiting_to_play = -1;
  }
}

void print_outcome(int player, int move, int result) {
  bwprintf(COM2, "Player %d played: %s, Result: %s\n", player,
           move_to_string(move), result_to_string(result));
}

void play_round(int p1, int p2) {
  int p1_move = moves[p1];
  int p2_move = moves[p2];

  moves[p1] = -1;
  moves[p2] = -1;

  int p1_outcome = move_outcomes[p1_move][p2_move];
  int p2_outcome = move_outcomes[p2_move][p1_move];

  reply(p1, p1_outcome);
  reply(p2, p2_outcome);

  bwprintf(COM2, "\nRound Complete\n");
  print_outcome(p1, p1_move, p1_outcome);
  print_outcome(p2, p2_move, p2_outcome);
  bwprintf(COM2, "Enter character to continue to next round...\n\n");
  bwgetc(COM2);
}

void _play(int tid, int move) {
  if (!is_valid_move(move)) {
    reply(tid, INVALID_MOVE);
  } else if (players[tid] == -1) {
    reply(tid, NO_GAME_IN_PROGRESS);
  } else {
    int opponent = players[tid];
    moves[tid] = move;
    if (moves[opponent] == -2) {
      reply(tid, PLAYER_QUIT);
      moves[tid] = -1;
      moves[opponent] = -1;
      players[tid] = -1;
    } else if (moves[opponent] != -1) {
      play_round(tid, opponent);
    }
  }
}

void _quit(int tid) {
  if (players[tid] == -1) {
    reply(tid, NOT_PLAYING);
  } else {
    moves[tid] = -1;
    reply(tid, SUCCESSFUL_QUIT);

    int opponent = players[tid];
    if (moves[opponent] != -2) {
      if (moves[opponent] != -1) {
        reply(opponent, PLAYER_QUIT);
      } else {
        moves[tid] = -2;
      }
    } else {
      moves[opponent] = -1;
    }
    players[tid] = -1;
  }
}

void rps_server_run() {
  RegisterAs("RPS Server");

  int tid;
  rps_request req;
  while (1) {
    Receive(&tid, (char *)&req, sizeof(rps_request));
    switch (req.type) {
      case SIGNUP:
        _signup(tid);
        break;
      case PLAY:
        _play(tid, req.move);
        break;
      case QUIT:
        _quit(tid);
        break;
      default:
        bwprintf(COM2, "Invalid rps request type %d\n", req.type);
    }
  }
}

/* Public Methods */

void start_rps_server() {
	bwsetfifo(COM2, OFF);
  waiting_to_play = -1;

  int i;
  for (i = 0; i < MAX_TASKS + 1; i++) {
    players[i] = -1;
    moves[i] = -1;
  }

  move_outcomes[ROCK][ROCK] = TIE;
  move_outcomes[ROCK][PAPER] = LOSE;
  move_outcomes[ROCK][SCISSORS] = WIN;

  move_outcomes[PAPER][ROCK] = WIN;
  move_outcomes[PAPER][PAPER] = TIE;
  move_outcomes[PAPER][SCISSORS] = LOSE;

  move_outcomes[SCISSORS][ROCK] = LOSE;
  move_outcomes[SCISSORS][PAPER] = WIN;
  move_outcomes[SCISSORS][SCISSORS] = TIE;

  Create(MED_PRI, &rps_server_run);
}

int get_rps_tid() {
  return WhoIs("RPS Server");
}

int signup(int tid) {
  rps_request req;
  req.type = SIGNUP;

  int reply;
  Send(tid, (char *)&req, sizeof(rps_request), (char *)&reply, sizeof(int));

  return reply;
}

int play(int tid, int move) {
  rps_request req;
  req.type = PLAY;
  req.move = move;

  int reply;
  Send(tid, (char *)&req, sizeof(rps_request), (char *)&reply, sizeof(int));

  return reply;
}

int quit(int tid) {
  rps_request req;
  req.type = QUIT;

  int reply;
  Send(tid, (char *)&req, sizeof(rps_request), (char *)&reply, sizeof(int));

  return reply;
}

