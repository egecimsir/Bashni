#ifndef PERFORMCONNECTION_H_
#define PERFORMCONNECTION_H_ "header guard"

#include "./sharedMemory.h"

int performConnection(
    int socket_fd,
    game_info_s *game_info,
    int sem_id,
    shared_info_s *info);

#endif  // PERFORMCONNECTION_H_
