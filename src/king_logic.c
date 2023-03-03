#include <stdio.h>

#include "./thinker_common.h"
#include "./king_logic.h"

direction can_king_capture(
    char board[ROWS][COLS][PCS],
    int row, int column,
    int enemy_man, int enemy_king,
    int previous_direction) {
#ifdef DEBUG
  printf("[DEBUG]: THINKER: checking if king %c%c\n can capture",
      int_column_to_char(column),
      int_row_to_char(row));
#endif

  // to top right
  if (BOT_LEFT != previous_direction) {
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
    } else {
      for (int i = 1; ROWS > row+i+2 && COLS > column+i+2; i++) {
        if ('*' == board[row+i][column+i][0]) {
          if (enemy_man == board[row+i+1][column+i+1][0] ||
             enemy_king == board[row+i+1][column+i+1][0]) {
#ifdef DEBUG
            puts("[DEBUG]: THINKER: opponent piece top right");
#endif
            if ('*' == board[row+i+2][column+i+2][0]) {
#ifdef DEBUG
              puts("[DEBUG]: THINKER: can capture opponent piece top right");
#endif
              return TOP_RIGHT;
            }
            break;
          }
          if ('*' == board[row+i+1][column+i+1][0]) {
            continue;
          }
        }
        break;
      }
    }
  }

  // to bot left
  if (TOP_RIGHT != previous_direction) {
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
    } else {
      for (int i = 1; 0  <= row-i-2 && 0 <= column-i-2; i++) {
        if ('*' == board[row-i][column-i][0]) {
          if (enemy_man == board[row-i-1][column-i-1][0] ||
             enemy_king == board[row-i-1][column-i-1][0]) {
#ifdef DEBUG
            puts("[DEBUG]: THINKER: opponent piece bot left");
#endif
            if ('*' == board[row-i-2][column-i-2][0]) {
#ifdef DEBUG
              puts("[DEBUG]: THINKER: can capture opponent piece bot left");
#endif
              return BOT_LEFT;
            }
            break;
          }
          if ('*' == board[row-i-1][column-i-1][0]) {
            continue;
          }
        }
        break;
      }
    }
  }

  // to bot right
  if (TOP_LEFT != previous_direction) {
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
    } else {
      for (int i = 1; 0  <= row-i-2 && COLS > column+i+2; i++) {
        if ('*' == board[row-i][column+i][0]) {
          if (enemy_man == board[row-i-1][column+i+1][0] ||
             enemy_king == board[row-i-1][column+i+1][0]) {
#ifdef DEBUG
            puts("[DEBUG]: THINKER: opponent piece bot right");
#endif
            if ('*' == board[row-i-2][column+i+2][0]) {
#ifdef DEBUG
              puts("[DEBUG]: THINKER: can capture opponent piece bot right");
#endif
              return BOT_RIGHT;
            }
            break;
          }
          if ('*' == board[row-i-1][column+i+1][0]) {
            continue;
          }
        }
        break;
      }
    }
  }

  // to top left
  if (BOT_RIGHT != previous_direction) {
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
    } else {
      for (int i = 1; ROWS > row+i+2 && 0 <= column-i-2; i++) {
        if ('*' == board[row+i][column-i][0]) {
          if (enemy_man == board[row+i+1][column-i-1][0] ||
             enemy_king == board[row+i+1][column-i-1][0]) {
#ifdef DEBUG
            puts("[DEBUG]: THINKER: opponent piece top left");
#endif
            if ('*' == board[row+i+2][column-i-2][0]) {
#ifdef DEBUG
              puts("[DEBUG]: THINKER: can capture opponent piece top left");
#endif
              return TOP_LEFT;
            }
            break;
          }
          if ('*' == board[row+i+1][column-i-1][0]) {
            continue;
          }
        }
        break;
      }
    }
  }
#ifdef DEBUG
  puts("[DEBUG]: THINKER: king cannot capture");
#endif

  return NO_DIR;
}

direction can_king_move(char board[ROWS][COLS][PCS],
    int row, int column) {
#ifdef DEBUG
  printf("[DEBUG]: THINKER: checking if king %c%c\n can move",
      int_column_to_char(column),
      int_row_to_char(row));
#endif
  if (0 <= row-1 && 0 <= column-1 &&
      '*' == board[row-1][column-1][0]) {
#ifdef DEBUG
    puts("[DEBUG]: THINKER: king can move to bottom left");
#endif
    return BOT_LEFT;
  }
  // check bottom right
  if (0 <= row-1 && COLS > column+1 &&
      '*' == board[row-1][column+1][0]) {
#ifdef DEBUG
    puts("[DEBUG]: THINKER: king can move to bottom right");
#endif
    return BOT_RIGHT;
  }
  // check top left
  if (ROWS > row+1 && 0 <= column-1 &&
      '*' == board[row+1][column-1][0]) {
#ifdef DEBUG
    puts("[DEBUG]: THINKER: king can move to top left");
#endif
    return TOP_LEFT;
  }
  // check top right
  if (ROWS > row+1 && COLS > column+1 &&
      '*' == board[row+1][column+1][0]) {
#ifdef DEBUG
    puts("[DEBUG]: THINKER: king can move to top right");
#endif
    return TOP_RIGHT;
  }

#ifdef DEBUG
  puts("[DEBUG]: THINKER: king cannot move");
#endif
  return NO_DIR;
}

int append_target_for_king_capture(
    char *move, direction dir,
    int *orig_row, int *orig_column,
    char board[ROWS][COLS][PCS],
    char enemy_man, char enemy_king) {
  char row = *orig_row;
  char col = *orig_column;
  switch (dir) {
    case TOP_LEFT:
      for (int i = 0; ROWS > row+i+1 && 0 <= col-i-1; i++) {
        if (enemy_man == board[row+i][col-i][0] ||
           enemy_king == board[row+i][col-i][0]) {
          board[row+i][col-i][0] = '*';
          *orig_row = row+i+1;
          *orig_column = col-i-1;
          break;
        }
      }
      return snprintf(move, MAX_MOVE_SIZE, ":%c%c",
          int_column_to_char(*orig_column),
          int_row_to_char(*orig_row));
    case TOP_RIGHT:
      for (int i = 0; ROWS > row+i+1 && COLS > col+i+1; i++) {
        if (enemy_man == board[row+i][col+i][0] ||
           enemy_king == board[row+i][col+i][0]) {
          board[row+i][col+i][0] = '*';
          *orig_row = row+i+1;
          *orig_column = col+i+1;
          break;
        }
      }
      return snprintf(move, MAX_MOVE_SIZE, ":%c%c",
          int_column_to_char(*orig_column),
          int_row_to_char(*orig_row));
    case BOT_RIGHT:
      for (int i = 0; 0 <= row-i-1 && COLS > col+i+1; i++) {
        if (enemy_man == board[row-i][col+i][0] ||
           enemy_king == board[row-i][col+i][0]) {
          board[row-i][col+i][0] = '*';
          *orig_row = row-i-1;
          *orig_column = col+i+1;
          break;
        }
      }
      return snprintf(move, MAX_MOVE_SIZE, ":%c%c",
          int_column_to_char(*orig_column),
          int_row_to_char(*orig_row));
    case BOT_LEFT:
      for (int i = 0; 0 <= row-i-1 && 0 <= col-i-1; i++) {
        if (enemy_man == board[row-i][col-i][0] ||
           enemy_king == board[row-i][col-i][0]) {
          board[row-i][col-i][0] = '*';
          *orig_row = row-i-1;
          *orig_column = col-i-1;
          break;
        }
      }
      return snprintf(move, MAX_MOVE_SIZE, ":%c%c",
          int_column_to_char(*orig_column),
          int_row_to_char(*orig_row));
    default:
      break;
  }
  return 0;
}

int create_king_capture_string(
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
    offset += append_target_for_king_capture(
      move+offset, capture_dir, &row, &column, board,
      enemy_man, enemy_king);
  } while (NO_DIR != (capture_dir = can_king_capture(
          board, row, column, enemy_man, enemy_king, capture_dir)));

  return 0;
}
