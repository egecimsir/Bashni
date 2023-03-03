#ifndef THINKER_H_
#define THINKER_H_ "header guard"

#include "./sharedMemory.h"

int think(int sem_id, shared_info_s *shared_info);

#endif  // THINKER_H_
