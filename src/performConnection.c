#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "./common.h"
#include "./network.h"
#include "./sharedMemory.h"
#include "./performConnection.h"
#include "./prologPhase.h"
#include "./gamePhase.h"

// Print human readable information about the current
// game session.
void print_game_info(game_info_s *game_info) {
  char *opponent_ready_str;
  if (game_info->opponent_ready) {
    opponent_ready_str = "bereit";
  } else {
    opponent_ready_str = "nicht bereit";
  }

  puts("Game session set up successfully!");
  puts("╔═══>");
  printf("║ Lobby-Name: %s\n",
      game_info->lobby_name);
  printf("║ Game-ID:    %s\n",
      game_info->game_id);
  printf("║ Lokal  (Spieler %d): %s\n",
      game_info->player_slot+1,
      game_info->player_name);
  printf("║ Gegner (Spieler %d): %s (%s)\n",
      game_info->opponent+1,
      game_info->opponent_name,
      opponent_ready_str);
  puts("║");
  printf(
      "║ URL to lobby:\n║ http://sysprak.priv.lab.nm.ifi.lmu.de/bashni/#%s\n",
      game_info->game_id);
  puts("╚═══>");
}

// iterate over predefined protocol steps (see defines in prolog.c)
int handle_prolog_phase(
    int socket_fd,
    game_info_s *game_info,
    char buf[BUFSIZE]) {
  int msg_len, answ_len;

  for (int i = 1; i < 9; i++) {
    msg_len = read_next_message(socket_fd, buf);

#ifdef DEBUG
    printf("[DEBUG]: CONNECTOR: server message:\n%9s%s", " ", buf);
#endif

    if (0 != parse_and_validate_prolog_message(i, msg_len, buf, game_info)) {
      return 1;
    }

    answ_len = construct_prolog_answer(i, buf, game_info);

    if (0 < answ_len) {
#ifdef DEBUG
      printf("[DEBUG]: CONNECTOR: constructed answer:\n%9s%s", " ", buf);
#endif
      send(socket_fd, buf, answ_len, 0);
    }
  }

  return 0;
}

int performConnection(
    int socket_fd,
    game_info_s *game_info,
    int sem_id,
    shared_info_s *shared_info) {
  char buf[BUFSIZE];
  shared_info_s info;

  memset(buf, 0, BUFSIZE * sizeof(char));

#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: >> Start of prolog phase <<\n");
#endif
  if (handle_prolog_phase(socket_fd, game_info, buf)) {
    fprintf(stderr, "error during prolog phase!\n");

    return 1;
  }
#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: >> End of prolog phase <<\n");
#endif

  print_game_info(game_info);
  initialize_info(game_info, &info);

  lock(sem_id);
  pass_struct_to_shm(&info, shared_info);
  unlock(sem_id);

#ifdef DEBUG
  lock(sem_id);
  print_shm_content(shared_info, "CONNECTOR");
  unlock(sem_id);
#endif

#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: >> Start of game phase <<\n");
#endif
  if (handle_game_phase(socket_fd, buf, sem_id, shared_info)) {
    fprintf(stderr, "error during game phase!\n");

    return 1;
  }
#ifdef DEBUG
  printf("[DEBUG]: CONNECTOR: >> End of game phase <<\n");
#endif

  return 0;
}
