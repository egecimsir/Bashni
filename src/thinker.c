#include <stdio.h>
#include <stdint.h>

#include "./common.h"
#include "./sharedMemory.h"
#include "./gamePhase.h"
#include "./thinker_common.h"
#include "./man_logic.h"
#include "./king_logic.h"
#include "./thinker.h"

void initialize_board(
    char board[ROWS][COLS][PCS],
    int sem_id,
    shared_info_s *shared_info,
    piece_s *piece) {
  for (int i = 0; i < ROWS; i++) {
    for (int k = 0; k < COLS; k++) {
      board[i][k][0] = '*';
    }
  }

  lock(sem_id);
  for (u_int64_t i = 0; i < shared_info->pieces_count; i++) {
    int row = char_row_to_int(piece->row);
    int column = char_column_to_int(piece->column);

    for (int z = PCS-1; z > 0; z--) {
      board[row][column][z] = board[row][column][z-1];
    }
    board[row][column][0] = piece->color;

    piece++;
  }
  unlock(sem_id);
}

int create_simple_move_string(
    char move[MAX_MOVE_SIZE],
    direction dir, int row, int column) {
  switch (dir) {
    case TOP_LEFT:
      return snprintf(move, MAX_MOVE_SIZE, "%c%d:%c%d",
          int_column_to_char(column), row+1,
          int_column_to_char(column-1), row+2);
    case TOP_RIGHT:
      return snprintf(move, MAX_MOVE_SIZE, "%c%d:%c%d",
          int_column_to_char(column), row+1,
          int_column_to_char(column+1), row+2);
    case BOT_RIGHT:
      return snprintf(move, MAX_MOVE_SIZE, "%c%d:%c%d",
          int_column_to_char(column), row+1,
          int_column_to_char(column+1), row);
    case BOT_LEFT:
      return snprintf(move, MAX_MOVE_SIZE, "%c%d:%c%d",
          int_column_to_char(column), row+1,
          int_column_to_char(column-1), row);
    default:
      return 0;
  }

  return 0;
}

int get_move_sequence(
    char board[ROWS][COLS][PCS],
    int player_color,
    char move[MAX_MOVE_SIZE]) {
  char color_man, color_king, enemy_man, enemy_king;
  direction dir;

  if (player_color) {
    color_man = 'b';
    color_king = 'B';
    enemy_man = 'w';
    enemy_king = 'W';
  } else {
    color_man = 'w';
    color_king = 'W';
    enemy_man = 'b';
    enemy_king = 'B';
  }

  // check which piece can capture
  for (int row = 0; row < ROWS; row++) {
    for (int column = 0; column < COLS; column++) {
      if (color_man == board[row][column][0] ||
          color_king == board[row][column][0]) {
#ifdef DEBUG
        printf("[DEBUG]: THINKER: checking %c%c for possible capture\n",
            int_column_to_char(column),
            int_row_to_char(row));
#endif
        if (color_king == board[row][column][0]) {
          if (NO_DIR != (dir = can_king_capture(
                  board, row, column, enemy_man, enemy_king, NO_DIR))) {
            return create_king_capture_string(
                move, dir, row, column,
                board, enemy_man, enemy_king);
            }
        }
        if (color_man == board[row][column][0]) {
          if (NO_DIR != (dir = can_man_capture(
                  board, row, column, enemy_man, enemy_king))) {
            return create_man_capture_string(
                move, dir, row, column,
                board, enemy_man, enemy_king);
          }
        }
      }
    }
  }

  // check which piece can make a simple move
  for (int row = 0; row < ROWS; row++) {
    for (int column = 0; column < COLS; column++) {
      if (color_man == board[row][column][0]) {
#ifdef DEBUG
        printf("[DEBUG]: THINKER: checking man %c%c for possible move\n",
            int_column_to_char(column),
            int_row_to_char(row));
#endif
        if (NO_DIR != (dir = can_man_move(board, row, column, player_color))) {
          return create_simple_move_string(move, dir, row, column);
        }
      }
      if (color_king == board[row][column][0]) {
#ifdef DEBUG
        printf("[DEBUG]: THINKER: checking king %c%c for possible move\n",
            int_column_to_char(column),
            int_row_to_char(row));
#endif
        if (NO_DIR != (dir = can_king_move(board, row, column))) {
          return create_simple_move_string(move, dir, row, column);
        }
      }
    }
  }

  return 0;
}

void calculate_move_sequence(
    char board[ROWS][COLS][PCS],
    int sem_id, shared_info_s *shared_info,
    char move[MAX_MOVE_SIZE]) {
  lock(sem_id);
  int player_color = shared_info->player_slot;
  unlock(sem_id);
#ifdef DEBUG
  if (player_color) {
    puts("[DEBUG]: THINKER: color of our pieces is black");
  } else {
    puts("[DEBUG]: THINKER: color of our pieces is white");
  }
#endif

  get_move_sequence(board, player_color, move);
}

void print_board(char board[ROWS][COLS][PCS]) {
  puts("\n   A B C D E F G H   ");
  puts(" +-----------------+ ");
  for (int i = ROWS-1; i >= 0; i--) {
    printf("%d|", i+1);
    for (int j = 0; j < 8; j++) {
      printf(" %c", board[i][j][0]);
    }
    printf(" |%d\n", i+1);
  }
  puts(" +-----------------+ ");
  puts("   A B C D E F G H   \n");
}

#ifdef DEBUG
void print_towers(char board[ROWS][COLS][PCS]) {
  puts("White Towers");
  puts("============");

  for (int row = 0; row < ROWS; row++) {
    for (int column = 0; column < COLS; column++) {
      if ('w' == board[row][column][0] || 'W' == board[row][column][0]) {
        printf("%c%d: ", int_column_to_char(column), row+1);
        for (int z = 0; z < PCS; z++) {
          if ('*' == board[row][column][z]) {
            break;
          }
          printf("%c", board[row][column][z]);
        }
        puts("");
      }
    }
  }
  puts("");
  puts("Black Towers");
  puts("============");
  for (int row = 0; row < ROWS; row++) {
    for (int column = 0; column < COLS; column++) {
      if ('b' == board[row][column][0] || 'B' == board[row][column][0]) {
        printf("%c%d: ", int_column_to_char(column), row+1);
        for (int z = 0; z < PCS; z++) {
          if ('*' == board[row][column][z]) {
            break;
          }
          printf("%c", board[row][column][z]);
        }
        puts("");
      }
    }
  }
  puts("");
}
#endif

int think(int sem_id, shared_info_s *shared_info) {
  char board[ROWS][COLS][PCS];
  char move[MAX_MOVE_SIZE];
  int pipe_fd;
  piece_s *piece, *piece_orig;

  lock(sem_id);

  if (NULL == (piece = (piece_s *)
        attach_shared_memory_segment(shared_info->board_shm_id))) {
    unlock(sem_id);
    return 1;
  }
  piece_orig = piece;  // save base pointer to pieces
  pipe_fd = shared_info->pipe_write_end;
#ifdef DEBUG
  u_int64_t count = shared_info->pieces_count;
#endif

  unlock(sem_id);

#ifdef DEBUG
  printf("[DEBUG]: THINKER: board shm addr: %p\n", piece);

  for (u_int64_t i = 0; i < count; i++) {
    print_piece_content(piece, "THINKER:");
    piece++;
  }
#endif

  initialize_board(board, sem_id, shared_info, piece_orig);

  print_board(board);
#ifdef DEBUG
  print_towers(board);
#endif

  calculate_move_sequence(board, sem_id, shared_info, move);
#ifdef DEBUG
  printf("[DEBUG]: THINKER: calculated move: '%s'\n", move);
#endif

  if (dprintf(pipe_fd, "%s", move) < 0) {
    fprintf(stderr, "failed to write to pipe\n");
    return 1;
  }

  if (detach_shared_memory_segment((void *) piece_orig)) {
    return 1;
  }
  return 0;
}
