#ifndef KING_LOGIC_H
#define KING_LOGIC_H "header guard"

#include "./thinker_common.h"

direction can_king_capture(
    char board[ROWS][COLS][PCS],
    int row, int column,
    int enemy_man, int enemy_king,
    int previous_direction);

direction can_king_move(char board[ROWS][COLS][PCS],
    int row, int column);

int append_target_for_king_capture(
    char *move, direction dir,
    int *orig_row, int *orig_column,
    char board[ROWS][COLS][PCS],
    char enemy_man, char enemy_king);

int create_king_capture_string(
    char move[MAX_MOVE_SIZE],
    direction dir, int row, int column,
    char board[ROWS][COLS][PCS],
    char enemy_man, char enemy_king);

#endif  // KING_LOGIC_H
