#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "./common.h"
#include "./prologPhase.h"

#define PROTOCOL_VERSION "2.3"

#define MSG_VERSION 1
#define MSG_ID 2
#define MSG_GAMEKIND 3
#define MSG_GAMENAME 4
#define MSG_PLAYERINFO 5
#define MSG_TOTAL 6
#define MSG_OPPONENT 7
#define MSG_ENDPLAYERS 8

// Returns 0 if the message "msg" is formatted correctly
// and has the expected content for the current protocol
// stage which is identified by "msg_type".
// Any positive value indicates that the conditions above
// where not met.
// The struct "game_info" is updated for server messages
// which provide the necessary information.
int parse_and_validate_prolog_message(
    int msg_type,
    int msg_len,
    char *msg,
    game_info_s *game_info) {
  char *expected;
  char *opponent_info;
  int major_version, minor_version;

  switch (msg_type) {
    case MSG_VERSION:
      if (2 != sscanf(msg, "+ MNM Gameserver v%d.%d",
            &major_version, &minor_version)) {
        fprintf(stderr, "Unexpected version message from server.\n");
        return 1;
      }
      if (2 != major_version) {
        fprintf(stderr, "Server uses unsupported protocol version.\n");
        return 1;
      }
      if (3 != minor_version) {
        fprintf(stderr,
            "Warning: Server is using different minor protocol version!\n");
      }
      memset(msg, 0, BUFSIZE * sizeof(char));
      return 0;

    case MSG_ID:
      expected = "+ Client version accepted - please send Game-ID to join\n";
      if (0 == strcmp(expected, msg)) {
        memset(msg, 0, BUFSIZE * sizeof(char));
        return 0;
      }
      fprintf(stderr, "Unexpected game id message from server.\n");
      return 1;

    case MSG_GAMEKIND:
      if (NULL == (expected = malloc((
                12 + strlen(game_info->gamekind_name)) * sizeof(char)))) {
        fprintf(stderr, "Could not allocate memory\n");
        return 2;
      }
      snprintf(expected,
          (12 + strlen(game_info->gamekind_name)) * sizeof(char),
          "+ PLAYING %s\n",
          game_info->gamekind_name);
      if (0 == strcmp(expected, msg)) {
        free(expected);
        memset(msg, 0, BUFSIZE * sizeof(char));
        return 0;
      }
      free(expected);
      // Check if validation failed because game kind was not bashni.
      expected = "+ PLAYING ";
      if (0 == strncmp(expected, msg, 10)) {
        fprintf(stderr, "Server game kind is not bashni.\n");
        return 1;
      }
      // Check if validation failed because the game ID was invalid
      expected = "- Not a valid game ID\n";
      if (0 == strcmp(expected, msg)) {
        fprintf(stderr, "The provided game ID is invalid!\n");
        return 1;
      }
      // Check if validation failed because the game ID does not exist.
      expected = "- Game does not exist\n";
      if (0 == strcmp(expected, msg)) {
        fprintf(stderr, "The provided game ID does not exist!\n");
        return 1;
      }
      fprintf(stderr, "Unexpected gamekind message from server.\n");
      return 1;

    case MSG_GAMENAME:
      if (NULL == (game_info->lobby_name = calloc(sizeof(char), msg_len))) {
        fprintf(stderr, "Could not allocate memory\n");
        return 2;
      }
      if (1 == sscanf(msg, "+ %[^\n]\n", game_info->lobby_name)) {
        memset(msg, 0, BUFSIZE * sizeof(char));
        return 0;
      }
      fprintf(stderr, "Unexpected gamekind message from server.\n");
      return 1;

    case MSG_PLAYERINFO:
      expected = "- No free player\n";
      if (0 == strcmp(expected, msg)) {
        fprintf(stderr, "There is no free player slot in this session.\n");
        return 1;
      }
      if (NULL == (game_info->player_name = calloc(sizeof(char), msg_len))) {
        fprintf(stderr, "Could not allocate memory\n");
        return 2;
      }
      if (2 == sscanf(msg, "+ YOU %d %[^\n]\n",
            &game_info->player_slot,
            game_info->player_name)) {
        memset(msg, 0, BUFSIZE * sizeof(char));
        return 0;
      }
      fprintf(stderr, "Unexpected player info message from server.\n");
      return 1;

    case MSG_TOTAL:
      expected = "+ TOTAL 2\n";
      if (0 == strcmp(expected, msg)) {
        memset(msg, 0, BUFSIZE * sizeof(char));
        return 0;
      }
      fprintf(stderr, "Unexpected message about total number of players\n");
      return 1;

    case MSG_OPPONENT:
      if (NULL == (opponent_info = calloc(sizeof(char), msg_len))) {
        fprintf(stderr, "Could not allocate memory\n");
        return 2;
      }
      if (2 == sscanf(msg, "+ %d %[^\n]\n",
            &game_info->opponent,
            opponent_info)) {
        if (1 == sscanf(opponent_info+(strlen(opponent_info)-1), "%d",
              &game_info->opponent_ready)) {
          opponent_info[strlen(opponent_info)-2] = '\0';
          game_info->opponent_name = opponent_info;
          memset(msg, 0, BUFSIZE * sizeof(char));
          return 0;
        }
      }
      free(opponent_info);
      fprintf(stderr, "Unexpected opponent info message from server.\n");
      return 1;

    case MSG_ENDPLAYERS:
      expected = "+ ENDPLAYERS\n";
      if (0 == strcmp(expected, msg)) {
        memset(msg, 0, BUFSIZE * sizeof(char));
        return 0;
      }
      fprintf(stderr, "Unexpected message after opponent list\n");
      return 1;

    default:
      fprintf(stderr, "unknown message type. unable to validate\n");
      return 1;
  }
}

int construct_prolog_answer(
    int msg_type,
    char buf[BUFSIZE],
    game_info_s *game_info) {
  memset(buf, 0, BUFSIZE);

  switch (msg_type) {
    case MSG_VERSION:
      return snprintf(buf, BUFSIZE, "VERSION "PROTOCOL_VERSION"\n");
    case MSG_ID:
      return snprintf(buf, BUFSIZE, "ID %s\n", game_info->game_id);
    case MSG_GAMEKIND:
      switch (game_info->player_slot) {
        case 0:
          return snprintf(buf, BUFSIZE, "PLAYER\n");
        default:
          return snprintf(
              buf,
              BUFSIZE,
              "PLAYER %d\n",
              game_info->player_slot - 1);
      }
    case MSG_GAMENAME:
    case MSG_PLAYERINFO:
    case MSG_TOTAL:
    case MSG_OPPONENT:
    case MSG_ENDPLAYERS:
      // no response
      return 0;
    default:
      // This should never™ be reached
      fprintf(stderr,
          "unknown message type. unable to construct answer\n");
      exit(EXIT_FAILURE);
  }
  return 0;
}
