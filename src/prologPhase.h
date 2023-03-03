#ifndef PROLOGPHASE_H_
#define PROLOGPHASE_H_ "header guard"

#include "./network.h"

int parse_and_validate_prolog_message(
    int msg_type,
    int msg_len,
    char *msg,
    game_info_s *game_info);

int construct_prolog_answer(
    int msg_type,
    char buf[BUFSIZE],
    game_info_s *game_info);

#endif   // PROLOGPHASE_H_
