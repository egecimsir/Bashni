#include <stdio.h>

#include "./thinker_common.h"
#include "./man_logic.h"

direction can_man_capture(
    char board[ROWS][COLS][PCS],
    int row, int column, int enemy_man, int enemy_king) {
#ifdef DEBUG
  printf("[DEBUG]: THINKER: checking if man %c%c\n can capture",
      int_column_to_char(column),
      int_row_to_char(row));
#endif

  if (ROWS > row+2 && 0 <= column-2 &&
      (enemy_man == board[row+1][column-1][0] ||
      enemy_king == board[row+1][column-1][0])) {
#ifdef DEBUG
    puts("[DEBUG]: THINKER: opponent piece top left");
#endif
    if ('*' == board[row+2][column-2][0]) {
#ifdef DEBUG
      puts("[DEBUG]: THINKER: can capture opponent piece top left");
#endif
      return TOP_LEFT;
    }
  }

  if (ROWS > row+2 && COLS > column+2 &&
      (enemy_man == board[row+1][column+1][0] ||
      enemy_king == board[row+1][column+1][0])) {
#ifdef DEBUG
    puts("[DEBUG]: THINKER: opponent piece top right");
#endif
    if ('*' == board[row+2][column+2][0]) {
#ifdef DEBUG
      puts("[DEBUG]: THINKER: can capture opponent piece top right");
#endif
      return TOP_RIGHT;
    }
  }

  if (0 <= row-2 && 0 <= column-2 &&
      (enemy_man == board[row-1][column-1][0] ||
      enemy_king == board[row-1][column-1][0])) {
#ifdef DEBUG
    puts("[DEBUG]: THINKER: opponent piece bot left");
#endif
    if ('*' == board[row-2][column-2][0]) {
#ifdef DEBUG
      puts("[DEBUG]: THINKER: can capture opponent piece bot left");
#endif
      return BOT_LEFT;
    }
  }

  if (0 <= row-2 && COLS > column+2 &&
      (enemy_man == board[row-1][column+1][0] ||
      enemy_king == board[row-1][column+1][0])) {
#ifdef DEBUG
    puts("[DEBUG]: THINKER: opponent piece bot right");
#endif
    if ('*' == board[row-2][column+2][0]) {
#ifdef DEBUG
      puts("[DEBUG]: THINKER: can capture opponent piece bot right");
#endif
      return BOT_RIGHT;
    }
  }

#ifdef DEBUG
  puts("[DEBUG]: THINKER: man cannot capture");
#endif
  return NO_DIR;
}

direction can_man_move(char board[ROWS][COLS][PCS],
    int row, int column, int own_color) {
#ifdef DEBUG
  printf("[DEBUG]: THINKER: checking if man %c%c\n can move",
      int_column_to_char(column),
      int_row_to_char(row));
#endif
  // black player
  if (own_color) {
    // check bottom left
    if (0 <= row-1 && 0 <= column-1 &&
        '*' == board[row-1][column-1][0]) {
#ifdef DEBUG
      puts("[DEBUG]: THINKER: man can move to bottom left");
#endif
      return BOT_LEFT;
    }
    // check bottom right
    if (0 <= row-1 && COLS > column+1 &&
        '*' == board[row-1][column+1][0]) {
#ifdef DEBUG
      puts("[DEBUG]: THINKER: man can move to bottom right");
#endif
      return BOT_RIGHT;
    }
#ifdef DEBUG
    puts("[DEBUG]: THINKER: man cannot move");
#endif
    return NO_DIR;
  }
  // white player
  // check top left
  if (ROWS > row+1 && 0 <= column-1 &&
      '*' == board[row+1][column-1][0]) {
#ifdef DEBUG
    puts("[DEBUG]: THINKER: man can move to top left");
#endif
    return TOP_LEFT;
  }
  // check top right
  if (ROWS > row+1 && COLS > column+1 &&
      '*' == board[row+1][column+1][0]) {
#ifdef DEBUG
    puts("[DEBUG]: THINKER: man can move to top right");
#endif
    return TOP_RIGHT;
  }
#ifdef DEBUG
    puts("[DEBUG]: THINKER: man cannot move");
#endif
  return NO_DIR;
}

int append_target_for_man_capture(
    char *move, direction dir,
    int *row, int *column,
    char board[ROWS][COLS][PCS]) {
  int ret = 0;
  switch (dir) {
    case TOP_LEFT:
      ret = snprintf(move, MAX_MOVE_SIZE, ":%c%d",
          int_column_to_char(*column-2), *row+3);
      board[*row+1][*column-1][0] = '*';
      *row = *row+2;
      *column = *column-2;
      break;
    case TOP_RIGHT:
      ret = snprintf(move, MAX_MOVE_SIZE, ":%c%d",
          int_column_to_char(*column+2), *row+3);
      board[*row+1][*column+1][0] = '*';
      *row = *row+2;
      *column = *column+2;
      break;
    case BOT_RIGHT:
      ret = snprintf(move, MAX_MOVE_SIZE, ":%c%d",
          int_column_to_char(*column+2), *row-1);
      board[*row-1][*column+1][0] = '*';
      *row = *row-2;
      *column = *column+2;
      break;
    case BOT_LEFT:
      ret = snprintf(move, MAX_MOVE_SIZE, ":%c%d",
          int_column_to_char(*column-2), *row-1);
      board[*row-1][*column-1][0] = '*';
      *row = *row-2;
      *column = *column-2;
      break;
    default:
      break;
  }
  return ret;
}

int create_man_capture_string(
    char move[MAX_MOVE_SIZE],
    direction dir, int row, int column,
    char board[ROWS][COLS][PCS],
    char enemy_man, char enemy_king) {
  direction capture_dir = dir;

  move[0] = int_column_to_char(column);
  move[1] = int_row_to_char(row);
  int offset = 2;
  do {
#ifdef DEBUG
    printf("[DEBUG]: THINKER: current capture step is: %c%c",
          int_column_to_char(column),
          int_row_to_char(row));
#endif
    offset += append_target_for_man_capture(
      move+offset, capture_dir, &row, &column, board);
  } while (NO_DIR != (capture_dir = can_man_capture(
          board, row, column, enemy_man, enemy_king)));

  return 0;
}

