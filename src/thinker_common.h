#ifndef THINKER_COMMON_H
#define THINKER_COMMON_H "header guard"

#define ROWS 8
#define COLS 8  // columns
#define PCS  25  // pieces
#define MAX_MOVE_SIZE 1000

typedef enum {
  TOP_LEFT,
  TOP_RIGHT,
  BOT_RIGHT,
  BOT_LEFT,
  NO_DIR,
} direction;

char int_column_to_char(int column);

char int_row_to_char(int row);

int char_row_to_int(char row);

int char_column_to_int(char column);

#endif  // THINKER_COMMON_H
