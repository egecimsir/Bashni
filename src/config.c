#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "./common.h"
#include "./config.h"

//  tries to match a given line and extract the correspoding value
//
//  returns 0 on success
int get_config_value(char *ret, char *line, size_t bufsize, const char *key) {
  char *match;
  if (NULL == (match = malloc(bufsize * sizeof(char)))) {
    fprintf(stderr, "could not allocate memory\n");
    return 1;
  }

  snprintf(match,
      (11 + strlen(key)) * sizeof(char),
      " %s = %%s\n",
      key);
#ifdef DEBUG
  printf("[DEBUG]: config line: %s%9smatch string: %s", line, " ", match);
#endif

  if (1 != sscanf(line, match, ret)) {
    fprintf(stderr, "unable to parse config value for %s!\n", key);
    free(match);
    return 1;
  }
#ifdef DEBUG
  printf("[DEBUG]: parsed value: %s\n", ret);
#endif

  free(match);
  return 0;
}

//  reads the filestream line by line with getline
//  checks for defined keys in every line
//  passes line to get_config_value for extraction
//  assigns values to corresponding fields in config
//
//  returns 0 on success
int parse_config_to_struct(FILE *filestream, game_info_s *config) {
  char *frbuffer, *value;
  size_t buffersize = 100;
  if (NULL == (frbuffer = malloc(buffersize * sizeof(char)))) {
    fprintf(stderr, "could not allocate memory\n");
    return 1;
  }

  // we currently expect a valid config file
  // to only contain 3 lines
  for (int i=1; i < 4; i++) {
    if (-1 == getline(&frbuffer, &buffersize, filestream)) {
      fprintf(stderr, "could not read from config file\n");
      free(frbuffer);
      return 1;
    }
    if (NULL == (value = calloc(buffersize, sizeof(char)))) {
      fprintf(stderr, "unable to allocate memory");
      free(frbuffer);
      return 1;
    }

    if (strstr(frbuffer, "portNumber") != 0) {
      if (get_config_value(value, frbuffer, buffersize, "portNumber")) {
        free(value);
        free(frbuffer);
        return 1;
      }
      config->port_number = atoi(value);
      free(value);
    } else if (strstr(frbuffer, "gameKindName") != 0) {
      if (get_config_value(value, frbuffer, buffersize, "gameKindName")) {
        free(value);
        free(frbuffer);
        return 1;
      }
      config->gamekind_name = value;
    } else if (strstr(frbuffer, "hostName") != 0) {
      if (get_config_value(value, frbuffer, buffersize, "hostName")) {
        free(value);
        free(frbuffer);
        return 1;
      }
      config->host_name = value;
    } else {
      fprintf(stderr, "Invalid config line %d!\n", i);
      free(value);
      free(frbuffer);
      return 1;
    }
    memset(frbuffer, 0, buffersize);
  }
  free(frbuffer);
  return 0;
}

//  checks if file specified in config_path exists
//  opens the file for reading with fopen
//  passes filestream to parse_config_to_struct for parsing
//  closes filestream after usage
//
//  returns 0 on success
int get_config_from_file(char *config_path, game_info_s *game_info) {
  FILE *filestream;
  struct stat _statbuf;

  if (stat(config_path, &_statbuf)) {
    printf("config file does not exist\n");
    return 1;
  }

  filestream = fopen(config_path, "r");
  if (NULL == filestream) {
     perror("Unable to read configuration file!\n");
     return 1;
  }

  if (parse_config_to_struct(filestream, game_info)) {
    fclose(filestream);
    fprintf(stderr, "failed to parse to struct");
    return 1;
  }

  fclose(filestream);
  return 0;
}

int get_game_info(game_info_s *game_info, int argc, char **argv) {
  char opt;
  char *game_id;
  char *player;
  char *config_file_path = "client.conf";  // default value
  memset(game_info, 0, sizeof(game_info_s));

  // read environment variables
  game_id = getenv("GAME_ID");

  if (NULL != game_id) {
    if (13 < strnlen(game_id, 14)) {
      fprintf(stderr, "The value of GAME_ID exceeds 13 characters!\n");
      // TODO(optional) should not fail if -g was specified (?)
      // specification does not define precedence of environment
      // variable and command-line parameter
      return 1;
    }
    game_info->game_id = game_id;
  }

  player = getenv("PLAYER");
  if (NULL != player) {
    game_info->player_slot = atoi(player);
  }

  // check command line parameters and overwrite
  // values passed via the environment
  while ((opt = getopt(argc, argv, "g:p:c:")) != -1) {
    switch (opt) {
      case 'g':
        if (13 < strnlen(optarg, 14)) {
          fprintf(stderr, "The value for -g exceeds 13 characters!\n");
          return 1;
        }
        game_info->game_id = optarg;
        break;
      case 'p':
        game_info->player_slot = atoi(optarg);
        break;
      case 'c':
        config_file_path = optarg;
        break;
      default:
        fprintf(stderr, "Unknown flag passed\n");
        return 1;
    }
  }

  if (get_config_from_file(config_file_path, game_info)) {
#ifdef DEBUG
    printf("[DEBUG]: failed to get config from file");
#endif
    exit(EXIT_FAILURE);
  }

  // validate game_info values
  if (NULL == game_info->game_id || 1 > strnlen(game_info->game_id, 13)) {
    fprintf(stderr, "No valid game id specified\n");
    return 1;
  }

  if (1 != game_info->player_slot && 2 != game_info->player_slot) {
#ifdef DEBUG
    printf("[DEBUG]: Player ID not 1 or 2. Will request a free player slot.\n");
#endif
    game_info->player_slot = 0;
  }

#ifdef DEBUG
  printf("[DEBUG]: game_info->player: %d\n%9sgame_info->game_id: %s\n",
      game_info->player_slot, " ",
      game_info->game_id);
#endif

  return 0;
}
