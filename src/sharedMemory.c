#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "./common.h"
#include "./sharedMemory.h"

// Returns the ID of the Semaphore or -1 if it fails
// semget() with constant NSEMS=1 and "SHM_R | SHM_W" as flag for read and write
// PS: NSEMS and flags could be taken as function parameters if neccesary...
int create_semaphore() {
  int ret, val, nsems;

  //  Semaphore set with a single semaphore
  nsems = 1;
  if (-1 == (ret = semget(IPC_PRIVATE, nsems, SHM_R | SHM_W))) {
    fprintf(stderr, "errno: %d\n", errno);
    perror("failed to get semaphore");

    return 1;
  }

  //  Initializes the single semaphore in the set to 1
  val = 1;
  if (-1 == (semctl(ret, 0, SETVAL, val))) {
    fprintf(stderr, "errno: %d\n", errno);
    perror("failed to initialize semaphore");

    return 1;
  }

  return ret;
}


// no error handling on failure because this
// function will only be called on program termination
void remove_semaphore(int sem_id) {
  struct seminfo _seminfo;

  if (-1 == (semctl(sem_id, 0, IPC_RMID, _seminfo))) {
    fprintf(stderr, "errno: %d\n", errno);
    perror("failed to remove semaphore");
  }

  return;
}

//  Locks the semaphore before a function reaches the critical area
void lock(int semid) {
  struct sembuf sem_op;
  sem_op.sem_num = 0;
  sem_op.sem_op = -1;
  sem_op.sem_flg = 0;

  semop(semid, &sem_op, 1);
}

//  Unlocks the semaphore
void unlock(int semid) {
  struct sembuf sem_op;
  sem_op.sem_num = 0;
  sem_op.sem_op = 1;
  sem_op.sem_flg = 0;

  semop(semid, &sem_op, 1);
}

//  shmget() with all parameters and error handling
int create_shared_memory_segment(size_t size) {
  int ret;

  if (-1 == (ret = shmget(IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | 0600))) {
    perror("failed to create new shared memory segment\n");

    return -1;
  }

  return ret;
}

//  Gets the address of the SHM and returns NULL in case of an error
void *attach_shared_memory_segment(int shmid) {
  void *ret;

  if ((void *) -1 == (ret = shmat(shmid, NULL, 0))) {
    fprintf(stderr, "errno: %d\n", errno);
    perror("failed to attach the shared memory segment");

    return NULL;
  }

  return ret;
}

int detach_shared_memory_segment(const void* shm_addr) {
  if (-1 == shmdt(shm_addr)) {
    fprintf(stderr, "errno: %d\n", errno);
    perror("failed to detach the shared memory segment");

    return 1;
  }

  return 0;
}

//  Writes the info struct into the Shared Memory
int pass_struct_to_shm(shared_info_s *info, shared_info_s *shm_addr) {
  if (memcpy(shm_addr, info, sizeof(shared_info_s)) != shm_addr) {
    fprintf(stderr, "failed to write into shared memory\n");

    return 1;
  }

  return 0;
}

// no error handling on failure because this
// function will only be called on program termination
void mark_segment_for_deletion(int semid) {
  if (-1 == shmctl(semid, IPC_RMID, NULL)) {
    fprintf(stderr, "errno: %d\n", errno);
    perror("failed to mark shared memory segment for deletion");
  }

  return;
}

// initializes infostruct that is later copied to shared memory
void initialize_info(game_info_s *game, shared_info_s *info) {
  memset(info, 0, sizeof(shared_info_s));
  strncpy(info->lobby_name, game->lobby_name, 1024);
  info->player_count = 2;
  info->player_slot = game->player_slot;

  // called from connector process
  info->conn_pid = getpid();
  info->think_pid = getppid();

  info->pipe_read_end = game->pipe_read_end;
  info->pipe_write_end = game->pipe_write_end;
}

#ifdef DEBUG
//  Tests if the Shared Memory is filled
int print_shm_content(shared_info_s *shm_addr, const char *prefix) {
  printf("[DEBUG]: %s: *shm_addr:         %p\n", prefix, shm_addr);
  printf("[DEBUG]: %s: PID:               %d\n", prefix, getpid());
  printf("[DEBUG]: %s: board info created:%d\n", prefix,
                                               shm_addr->board_info_ready);
  printf("[DEBUG]: %s: board info ready:  %d\n", prefix,
                                                 shm_addr->board_info_ready);
  printf("[DEBUG]: %s: thinker PID:       %d\n", prefix, shm_addr->think_pid);
  printf("[DEBUG]: %s: connector PID:     %d\n", prefix, shm_addr->conn_pid);
  printf("[DEBUG]: %s: player_slot:       %d\n", prefix, shm_addr->player_slot);
  printf("[DEBUG]: %s: player_count:      %d\n", prefix,
                                               shm_addr->player_count);
  printf("[DEBUG]: %s: board_shm_id:      %d\n", prefix,
                                               shm_addr->board_shm_id);
  printf("[DEBUG]: %s: move_time_msecs:   %lu\n", prefix,
                                                  shm_addr->move_time_msecs);
  printf("[DEBUG]: %s: pieces_count:      %lu\n", prefix,
                                                shm_addr->pieces_count);
  printf("[DEBUG]: %s: *lobby_name:       %p\n", prefix, shm_addr->lobby_name);
  printf("[DEBUG]: %s: lobby_name:        %s\n", prefix, shm_addr->lobby_name);

  return 0;
}
#endif
