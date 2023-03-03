#ifndef CONFIG_H_
#define CONFIG_H_ "header guard"

int get_config_from_file(char *config_path, game_info_s *game_info);
int get_game_info(game_info_s *game_info, int argc, char **argv);

#endif  // CONFIG_H_
