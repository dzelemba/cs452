#ifndef __RPS_SERVER_H__
#define __RPS_SERVER_H__

#define ROCK 0
#define PAPER 1
#define SCISSORS 2

// Signup response types.
#define OK 0
#define ALREADY_PLAYING 1

// Play response types.
#define WIN 0
#define LOSE 1
#define TIE 2
#define PLAYER_QUIT 3
#define NO_GAME_IN_PROGRESS 4
#define INVALID_MOVE 5

// Quit response types.
#define SUCCESSFUL_QUIT 0
#define NOT_PLAYING 1

void start_rps_server();

int get_rps_tid();

int signup(int tid);

int play(int tid, int move);

int quit(int tid);

#endif
