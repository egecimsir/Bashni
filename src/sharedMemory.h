#ifndef SHAREDMEMORY_H_
#define SHAREDMEMORY_H_ "header guard"
#include <sys/types.h>

typedef struct {
  int board_info_created;
  int board_info_ready;
  int pipe_read_end;
  int pipe_write_end;
  pid_t think_pid;
  pid_t conn_pid;
  int player_slot;
  int player_count;
  int board_shm_id;
  u_int64_t move_time_msecs;
  u_int64_t pieces_count;
  char lobby_name[1024];
} shared_info_s;

int create_semaphore();

void remove_semaphore(int sem_id);

void lock(int sem_id);

void unlock(int sem_id);

int create_shared_memory_segment(size_t size);

void *attach_shared_memory_segment(int shm_id);

int detach_shared_memory_segment(const void* shm_addr);

int pass_struct_to_shm(shared_info_s *info, shared_info_s *shm_addr);

void mark_segment_for_deletion(int sem_id);

void initialize_info(game_info_s *game, shared_info_s *info);

#ifdef DEBUG
int print_shm_content(shared_info_s *shm_addr, const char *prefix);
#endif

#endif  // SHAREDMEMORY_H_
