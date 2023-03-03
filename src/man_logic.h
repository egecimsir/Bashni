#ifndef MAN_LOGIC_H
#define MAN_LOGIC_H "header guard"

#include "./thinker_common.h"

direction can_man_capture(
    char board[ROWS][COLS][PCS],
    int row, int column, int enemy_man, int enemy_king);

direction can_man_move(char board[ROWS][COLS][PCS],
    int row, int column, int own_color);

int append_target_for_man_capture(
    char *move, direction dir,
    int *row, int *column,
    char board[ROWS][COLS][PCS]);

int create_man_capture_string(
    char move[MAX_MOVE_SIZE],
    direction dir, int row, int column,
    char board[ROWS][COLS][PCS],
    char enemy_man, char enemy_king);

#endif  // MAN_LOGIC_H
