#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "./common.h"
#include "./network.h"
#include "./gamePhase.h"
#include "./sharedMemory.h"

#ifdef DEBUG
// get string representation of each enum member
char *get_msg_type_as_str(game_msg_t msg_type) {
  switch (msg_type) {
    case WAIT:
      return "WAIT";
    case MOVE:
      return "MOVE";
    case PIECESLIST:
      return "PIECESLIST";
    case PIECEINFO:
      return "PIECEINFO";
    case ENDPIECESLIST:
      return "ENDPIECESLIST";
    case OKTHINK:
      return "OKTHINK";
    case GAMEOVER:
      return "GAMEOVER";
    case PLAYER0WON:
      return "PLAYER0WON";
    case PLAYER1WON:
      return "PLAYER1WON";
    case QUIT:
      return "QUIT";
    default:
    return "ERROR";
  }
}

// print information about a game piece
void print_piece_content(piece_s *piece, const char *prefix) {
  printf("[DEBUG]: %s {%s%c%s%c%s%c}\n",
         prefix,
         "color: ", piece->color,
         ",row: ", piece->row,
         ",column: ", piece->column);
}
#endif

// categorizes message and sets game_msg->msg_type accordingly
// validates format of received message in game_msg->msg
// extracts value from message and sets according field in game_msg
game_msg_s *parse_and_validate_game_message(game_msg_s *game_msg) {
  char *expected;
  piece_s *piece;

#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: msg_len: %d\n", game_msg->msg_len);
#endif

  expected = "- TIMEOUT";
  if (0 == strncmp(expected, game_msg->msg, strlen(expected))) {
    fprintf(stderr, "failed to answer before timeout\n");
    game_msg->msg_type = ERROR;

    return game_msg;
  }

  expected = "- Invalid Move";
  if (0 == strncmp(expected, game_msg->msg, strlen(expected))) {
    fprintf(stderr, "server rejected invalid move\n");
    game_msg->msg_type = ERROR;

    return game_msg;
  }

  expected = "- ";
  if (0 == strncmp(expected, game_msg->msg, 2)) {
    fprintf(stderr, "error message from server\n");
    game_msg->msg_type = ERROR;

    return game_msg;
  }

  expected = "+ WAIT\n";
  if (0 == strcmp(expected, game_msg->msg)) {
    game_msg->msg_type = WAIT;

    return game_msg;
  }

  if (1 == sscanf(game_msg->msg, "+ MOVE %lu\n", &(game_msg->int_val))) {
    game_msg->msg_type = MOVE;

    return game_msg;
  }

  if (1 == sscanf(game_msg->msg, "+ PIECESLIST %lu\n", &(game_msg->int_val))) {
    game_msg->msg_type = PIECESLIST;

    return game_msg;
  }

  if (NULL == (piece = malloc(sizeof(piece_s)))) {
    fprintf(stderr, "could not allocate memory\n");
    game_msg->msg_type = ERROR;

    return game_msg;
  }
  if (3 == sscanf(game_msg->msg, "+%*1[^\n]%c@%c%c\n",
        &(piece->color),
        &(piece->column),
        &(piece->row))) {
    switch (piece->color) {
      case 'w': case 'W': case 'b': case 'B':
        break;
      default:
        free(piece);
        game_msg->msg_type = ERROR;
        fprintf(stderr, "Invalid piece color received from server\n");

        return game_msg;
    }
    if ('A' > piece->column || 'H' < piece->column) {
        free(piece);
        game_msg->msg_type = ERROR;
        fprintf(stderr, "Invalid piece column received from server\n");

        return game_msg;
    }
    if ('1' > piece->row || '8' < piece->row) {
        free(piece);
        game_msg->msg_type = ERROR;
        fprintf(stderr, "Invalid piece row received from server\n");

        return game_msg;
    }

    // valid pieceinfo message
    game_msg->piece = piece;
    game_msg->msg_type = PIECEINFO;

    return game_msg;
  }
  free(piece);

  expected = "+ ENDPIECESLIST\n";
  if (0 == strcmp(expected, game_msg->msg)) {
    game_msg->msg_type = ENDPIECESLIST;

    return game_msg;
  }

  expected = "+ OKTHINK\n";
  if (0 == strcmp(expected, game_msg->msg)) {
    game_msg->msg_type = OKTHINK;

    return game_msg;
  }

  expected = "+ MOVEOK\n";
  if (0 == strcmp(expected, game_msg->msg)) {
    game_msg->msg_type = MOVEOK;

    return game_msg;
  }

  expected = "+ GAMEOVER\n";
  if (0 == strcmp(expected, game_msg->msg)) {
    game_msg->msg_type = GAMEOVER;

    return game_msg;
  }

  expected = "+ PLAYER0WON Yes\n";
  if (0 == strcmp(expected, game_msg->msg)) {
    game_msg->msg_type = PLAYER0WON;
    game_msg->str_val = "won the game!";

    return game_msg;
  }

  expected = "+ PLAYER0WON No\n";
  if (0 == strcmp(expected, game_msg->msg)) {
    game_msg->msg_type = PLAYER0WON;
    game_msg->str_val = "placed second! :)";

    return game_msg;
  }

  expected = "+ PLAYER1WON Yes\n";
  if (0 == strcmp(expected, game_msg->msg)) {
    game_msg->msg_type = PLAYER1WON;
    game_msg->str_val = "won the game!";

    return game_msg;
  }

  expected = "+ PLAYER1WON No\n";
  if (0 == strcmp(expected, game_msg->msg)) {
    game_msg->msg_type = PLAYER1WON;
    game_msg->str_val = "placed second! :)";

    return game_msg;
  }

  expected = "+ QUIT\n";
  if (0 == strcmp(expected, game_msg->msg)) {
    game_msg->msg_type = QUIT;

    return game_msg;
  }

  fprintf(stderr, "Unexpected message from server:\n%s", game_msg->msg);
  game_msg->msg_type = ERROR;
  return game_msg;
}

game_msg_s *parse_next_message(
    char buf[BUFSIZE],
    game_msg_s *game_msg,
    int socket_fd) {
  memset(buf, 0, BUFSIZE);
  memset(game_msg, 0, sizeof(game_msg_s));
  game_msg->msg_len = read_next_message(socket_fd, buf);
  game_msg->msg = buf;
#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: server message:\n%9s%s", " ", buf);
#endif
  game_msg = parse_and_validate_game_message(game_msg);

  return game_msg;
}

int construct_game_answer(
    char buf[BUFSIZE],
    game_msg_s *game_msg) {
  char *answer;

#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: constructing answer for message type %s\n",
      get_msg_type_as_str(game_msg->msg_type));
#endif

  if (WAIT == game_msg->msg_type) {
    answer = "OKWAIT\n";
    memset(buf, 0, BUFSIZE);

#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: constructed answer %s", answer);
#endif
    return snprintf(buf, BUFSIZE, "%s", answer);
  }

  if (ENDPIECESLIST == game_msg->msg_type) {
    answer = "THINKING\n";
    memset(buf, 0, BUFSIZE);

#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: constructed answer %s", answer);
#endif
    return snprintf(buf, BUFSIZE, "%s", answer);
  }

#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: no answer for message type %s\n",
      get_msg_type_as_str(game_msg->msg_type));
#endif
  memset(buf, 0, BUFSIZE);
  return 0;
}

int construct_move_answer(
    char buf[BUFSIZE],
    int sem_id,
    shared_info_s *shared_info) {
  char move[BUFSIZE-7];
  memset(move, 0, BUFSIZE-7);

  int n;

  lock(sem_id);
  if (-1 == (n = read(shared_info->pipe_read_end, move, BUFSIZE-7))) {
    fprintf(stderr, "failed to read move from pipe\n");
    return 0;
  }
  unlock(sem_id);

#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: read move from pipe '%s'\n", move);
#endif
  printf("Sending move %s to server\n", move);

  memset(buf, 0, BUFSIZE);
  return snprintf(buf, BUFSIZE, "PLAY %s\n", move);
}

int process_game_msg_content(
    game_msg_s *game_msg,
    int sem_id,
    shared_info_s *shared_info) {
  // server message contains max time in milliseconds for next move
  if (MOVE == game_msg->msg_type) {
    lock(sem_id);
    shared_info->move_time_msecs = game_msg->int_val;
    unlock(sem_id);
  }

  // server message contains number of pieces on the board
  if (PIECESLIST == game_msg->msg_type) {
    int board_shm_id;
    // create shared memory segment that fits the expected
    // amount of piece_s structs containing game piece info
    if (-1 == (board_shm_id =
          create_shared_memory_segment(sizeof(piece_s) * game_msg->int_val))) {
      return 1;
    }

    lock(sem_id);
    shared_info->pieces_count = game_msg->int_val;
    shared_info->board_shm_id = board_shm_id;
    shared_info->board_info_created = 1;
#ifdef DEBUG
    printf("[DEBUG]: expecting %lu pieces\n", shared_info->pieces_count);
#endif
    unlock(sem_id);

    return 0;
  }

  return 0;
}

int write_piece_info_to_shared_memory(
    game_msg_s *game_msg,
    u_int64_t piece_index,
    piece_s *piece_list) {
  if (piece_list+piece_index != memcpy(
        piece_list+piece_index , game_msg->piece, sizeof(piece_s))) {
    fprintf(stderr, "failed to write into shared memory\n");
    free(game_msg->piece);

    return 1;
  }
#ifdef DEBUG
  print_piece_content(piece_list+piece_index, "CONNECTOR:");
#endif

  free(game_msg->piece);
  return 0;
}

int handle_idle_sequence(char buf[BUFSIZE], game_msg_s *msg, int socket_fd) {
#ifdef DEBUG
  puts("[DEBUG]: CONNECTOR: --- BEGIN IDLE SEQUENCE ---");
#endif
  // set cursor to beginning of message to avoid
  // endlessly writing redundant waiting messages
  printf("Waiting for opponent...\r\n\033[1A");

  if (-1 == send(socket_fd, buf, construct_game_answer(buf, msg) , 0)) {
    fprintf(stderr, "failed to send answer to server\n");
    return 1;
  }

#ifdef DEBUG
  puts("[DEBUG]: CONNECTOR: --- END IDLE SEQUENCE ---");
#endif
  return 0;
}

int handle_pieceslist_sequence(
    char buf[BUFSIZE],
    game_msg_s *game_msg,
    int socket_fd,
    int sem_id,
    shared_info_s *shared_info) {
  piece_s *piece_list;
  u_int64_t total_pieces;
#ifdef DEBUG
  puts("[DEBUG]: CONNECTOR: --- BEGIN PIECESLIST SEQUENCE ---");
#endif

  // PIECESLIST
  game_msg = parse_next_message(buf, game_msg, socket_fd);

  if (PIECESLIST != game_msg->msg_type) {
    fprintf(stderr, "did not receive expected message with piece count\n");
    return 1;
  }
  if (process_game_msg_content(game_msg, sem_id, shared_info)) {
    return 1;
  }
  lock(sem_id);
  if (NULL == (piece_list = (piece_s *)
      attach_shared_memory_segment(shared_info->board_shm_id))) {
    mark_segment_for_deletion(shared_info->board_shm_id);
    unlock(sem_id);
    return 1;
  }
#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: successfully attached board shm: %p\n",
      piece_list);
#endif
  unlock(sem_id);

  total_pieces = game_msg->int_val;

  for (u_int64_t i = 0; i < total_pieces; i++) {
    // read msg
    game_msg = parse_next_message(buf, game_msg, socket_fd);

    // make sure its pieceinfo
    if (PIECEINFO != game_msg->msg_type) {
      fprintf(stderr,
          "did not receive expected message about game piece from server\n");
      return 1;
    }
    write_piece_info_to_shared_memory(game_msg, i, piece_list);
  }

  game_msg = parse_next_message(buf, game_msg, socket_fd);

  if (ENDPIECESLIST != game_msg->msg_type) {
    fprintf(stderr,
        "did not receive expected message following game piece info\n");
    return 1;
  }

#ifdef DEBUG
  puts("[DEBUG]: CONNECTOR: --- END PIECESLIST SEQUENCE ---");
#endif
  return 0;
}

int handle_move_sequence(
    char buf[BUFSIZE],
    game_msg_s *game_msg,
    int socket_fd,
    int sem_id,
    shared_info_s *shared_info) {
  int len;
#ifdef DEBUG
  puts("[DEBUG]: CONNECTOR: --- BEGIN MOVE SEQUENCE ---");
#endif

  // MOVE
  if (process_game_msg_content(game_msg, sem_id, shared_info)) {
    return 1;
  }

  if (handle_pieceslist_sequence(
        buf, game_msg, socket_fd, sem_id, shared_info)) {
    return 1;
  }
  lock(sem_id);
  shared_info->board_info_ready = 1;
  unlock(sem_id);
  if (-1 == send(socket_fd, buf, construct_game_answer(buf, game_msg) , 0)) {
    fprintf(stderr, "failed to send answer to server\n");
    return 1;
  }

  game_msg = parse_next_message(buf, game_msg, socket_fd);

  if (OKTHINK != game_msg->msg_type) {
    fprintf(stderr,
        "did not receive expected message following THINKING\n");
    return 1;
  }
  // <epoll>
  #define MAX_EVENTS 10
  struct epoll_event ev, events[MAX_EVENTS];
  memset(&ev, 0, sizeof(struct epoll_event));
  int nfds, epollfd, pipe_fd;

  lock(sem_id);
  pipe_fd = shared_info->pipe_read_end;
  unlock(sem_id);

  // wake up thinker process
  lock(sem_id);
  kill(shared_info->think_pid, SIGUSR1);
  unlock(sem_id);

  if (-1 == (epollfd = epoll_create1(0))) {
    fprintf(stderr, "failed create epoll instance\n");
    return 1;
  }

  ev.events = EPOLLIN | EPOLLHUP;
  ev.data.fd = socket_fd;
  if (-1 == epoll_ctl(epollfd, EPOLL_CTL_ADD, socket_fd, &ev)) {
    fprintf(stderr, "failed add socket to epoll\n");
    return 1;
  }

  ev.data.fd = pipe_fd;
  if (-1 == epoll_ctl(epollfd, EPOLL_CTL_ADD, pipe_fd, &ev)) {
    fprintf(stderr, "failed add pipe to epoll\n");
    return 1;
  }

  nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
  if (-1 == nfds) {
    fprintf(stderr, "epoll_wait failed\n");
    return 1;
  }
  // ++n used in manpage
  for (int n = 0; n < nfds; n++) {
    if (events[n].data.fd == socket_fd) {
      if (events[n].events & EPOLLHUP) {
        fprintf(stderr,
            "connection to server interrupted\n");
        return 1;
      }
      fprintf(stderr,
          "incoming message from server before move was sent\n");
      return 0;
    }
    if (events[n].data.fd == pipe_fd) {
      if (events[n].events & EPOLLHUP) {
        fprintf(stderr,
            "connection to server interrupted\n");
        return 1;
      }
#ifdef DEBUG
      printf("[DEBUG]: CONNECTOR: pipe ready for read\n");
#endif
      break;
    }
  }

  if (0 == (len = construct_move_answer(buf, sem_id, shared_info))) {
    return 1;
  }

  if (-1 == send(socket_fd, buf, len, 0)) {
    fprintf(stderr, "failed to send answer to server\n");
    return 1;
  }
#ifdef DEBUG
  puts("[DEBUG]: CONNECTOR: --- END MOVE SEQUENCE ---");
#endif
  return 0;
}

int handle_gameover_sequence(
    char buf[BUFSIZE],
    game_msg_s *game_msg,
    int socket_fd,
    int sem_id,
    shared_info_s *shared_info) {
#ifdef DEBUG
  puts("[DEBUG]: CONNECTOR: --- BEGIN GAMEOVER SEQUENCE ---");
#endif

  if (handle_pieceslist_sequence(
        buf, game_msg, socket_fd, sem_id, shared_info)) {
    return 1;
  }

  game_msg = parse_next_message(buf, game_msg, socket_fd);

  if (PLAYER0WON != game_msg->msg_type) {
    fprintf(stderr,
        "did not receive expected message about victory of first player\n");
    return 1;
  }
  lock(sem_id);
  if (!shared_info->player_slot) {
    printf("You %s \\o/ \\o/\n", game_msg->str_val);
  }
  unlock(sem_id);

  game_msg = parse_next_message(buf, game_msg, socket_fd);

  if (PLAYER1WON != game_msg->msg_type) {
    fprintf(stderr,
        "did not receive expected message about victory of second player\n");
    return 1;
  }
  lock(sem_id);
  if (shared_info->player_slot) {
    printf("You %s \\o/ \\o/\n", game_msg->str_val);
  }
  unlock(sem_id);

  game_msg = parse_next_message(buf, game_msg, socket_fd);

  if (QUIT != game_msg->msg_type) {
    fprintf(stderr,
        "did not receive expected message about end of session\n");
    return 1;
  }
#ifdef DEBUG
  puts("[DEBUG]: CONNECTOR: --- END GAMEOVER SEQUENCE ---");
#endif
  return 0;
}

int handle_game_phase(
    int socket_fd,
    char buf[BUFSIZE],
    int sem_id,
    shared_info_s *shared_info) {
  game_msg_s *game_msg;

  if (NULL == (game_msg = malloc(sizeof(game_msg_s)))) {
    fprintf(stderr, "could not allocate memory\n");
    return 1;
  }

  while (1) {
    game_msg = parse_next_message(buf, game_msg, socket_fd);

    switch (game_msg->msg_type) {
      case MOVEOK:
        printf("Server accepted move\n");
        break;
      case WAIT:
        if (handle_idle_sequence(buf, game_msg, socket_fd)) {
          free(game_msg);
          return 1;
        }
        break;
      case MOVE:
        if (handle_move_sequence(
              buf, game_msg, socket_fd, sem_id, shared_info)) {
          free(game_msg);
          return 1;
        }
        break;
      case GAMEOVER:
        if (handle_gameover_sequence(
              buf, game_msg, socket_fd, sem_id, shared_info)) {
          free(game_msg);
          return 1;
        }
        free(game_msg);
        return 0;
      case QUIT:
        fprintf(stderr,
            "session terminated by server due to protocol error\n");
        // fall through
      case ERROR:
      default:
        free(game_msg);
        return 1;
    }
  }
}
