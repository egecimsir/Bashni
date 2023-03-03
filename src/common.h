#ifndef COMMON_H_
#define COMMON_H_ "header guard"

#include <sys/types.h>

typedef struct {
  char *game_id;
  // player_slot may be 0, 1 or 2
  // 0: request a free slot
  // 1: First player (PLAYER 0)
  // 2: Second player (PLAYER 1)
  // after prolog phase:
  // 0 or 1 for the respective player slots
  int player_slot;
  // opponent may be 0 or 1
  int opponent;
  // opponent_ready may be 0 or 1
  int opponent_ready;
  char *lobby_name;
  char *player_name;
  char *opponent_name;
  char *host_name;
  uint16_t port_number;
  char *gamekind_name;
  int pipe_read_end;
  int pipe_write_end;
} game_info_s;

#endif  // COMMON_H_
