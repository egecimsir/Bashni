#include "./thinker_common.h"

char int_column_to_char(int column) {
  return (char) 65 + column;
}

char int_row_to_char(int row) {
  return (char) 49 + row;
}

int char_row_to_int(char row) {
  return (int) row - 49;
}

int char_column_to_int(char column) {
  return (int) column - 65;
}
