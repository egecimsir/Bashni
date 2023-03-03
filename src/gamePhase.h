#ifndef GAMEPHASE_H_
#define GAMEPHASE_H_ "header guard"
#include <sys/types.h>

#include "./network.h"
#include "./sharedMemory.h"

typedef enum {
  WAIT,
  MOVE,
  PIECESLIST,
  PIECEINFO,
  ENDPIECESLIST,
  OKTHINK,
  MOVEOK,
  GAMEOVER,
  PLAYER0WON,
  PLAYER1WON,
  QUIT,
  ERROR
} game_msg_t;

typedef enum {
  UINT64,
  STRING,
  PIECE,
  NONE
} game_msg_val_t;

typedef struct {
  char row;
  char column;
  char color;
} piece_s;

typedef struct {
  game_msg_t     msg_type;
  int            msg_len;
  char           *msg;
  game_msg_val_t val_type;
  u_int64_t      int_val;
  char           *str_val;
  piece_s        *piece;
} game_msg_s;

int handle_game_phase(
    int socket_fd,
    char buf[BUFSIZE],
    int sem_id,
    shared_info_s *shared_info);

#ifdef DEBUG
void print_piece_content(piece_s *piece, const char *prefix);
#endif

#endif  // GAMEPHASE_H_
