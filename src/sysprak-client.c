#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "./common.h"
#include "./config.h"
#include "./network.h"
#include "./gamePhase.h"
#include "./sharedMemory.h"
#include "./performConnection.h"
#include "./thinker.h"

void print_usage(const char *name) {
  fprintf(stderr, "Usage: %s -g GAME_ID -p [1|2] -c CONFIG_FILE\n", name);
}

// Free's all the above defined dynamic memory usage
// TODO(optional) improve cleanup
int cleanup(game_info_s *game_info) {
  if (NULL != game_info->lobby_name) {
    free(game_info->lobby_name);
  }
  if (NULL != game_info->player_name) {
    free(game_info->player_name);
  }
  if (NULL != game_info->opponent_name) {
    free(game_info->opponent_name);
  }
  if (NULL != game_info->gamekind_name) {
    free(game_info->gamekind_name);
  }
  if (NULL != game_info->host_name) {
    free(game_info->host_name);
  }

  return 0;
}

int main(int argc, char **argv) {
  int sfd, sem_id, shm_id, fd[2];
  pid_t pid;
  game_info_s game_info;
  shared_info_s *shared_info;

#ifdef DEBUG
  printf("[DEBUG]: Compiled with debug output enabled!\n");
#endif

  if (0 != get_game_info(&game_info, argc, argv)) {
    print_usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  sem_id = create_semaphore();
#ifdef DEBUG
  printf("[DEBUG]: semaphore id: %d\n", sem_id);
#endif

  shm_id = create_shared_memory_segment(sizeof(shared_info_s));
#ifdef DEBUG
  printf("[DEBUG]: shared memory segment id: %d\n", shm_id);
#endif

  // attach shared memory segment
  if (NULL == (shared_info =
        (shared_info_s *) attach_shared_memory_segment(shm_id))) {
    cleanup(&game_info);
    remove_semaphore(sem_id);
    mark_segment_for_deletion(shm_id);
    exit(EXIT_FAILURE);
  }

  // create unnamed pipe
  if (pipe(fd) < 0) {
    fprintf(stderr, "failed to create pipe\n");
    cleanup(&game_info);
    remove_semaphore(sem_id);
    mark_segment_for_deletion(shm_id);
    exit(EXIT_FAILURE);
  }

  game_info.pipe_read_end = fd[0];
  game_info.pipe_write_end = fd[1];

  // Spliting into two process' : Thinker and Connector
  switch (pid = fork()) {
    case -1:
      // Fork() malfunctioning
      fprintf(stderr, "failed to fork\n");
      cleanup(&game_info);
      remove_semaphore(sem_id);
      mark_segment_for_deletion(shm_id);
      close(game_info.pipe_read_end);
      close(game_info.pipe_write_end);
      exit(EXIT_FAILURE);

    case 0:
      // Connector
      if (-1 == close(game_info.pipe_write_end)) {
#ifdef DEBUG
        perror("failed to close write end of pipe\n");
#endif
        cleanup(&game_info);
        remove_semaphore(sem_id);
        exit(EXIT_FAILURE);
      }

      if (0 > (sfd = create_tcp_socket(
              game_info.host_name,
              game_info.port_number))) {
        fprintf(stderr, "Could not set up a TCP connection!\n");
        cleanup(&game_info);
        remove_semaphore(sem_id);
        exit(EXIT_FAILURE);
      }

      if (performConnection(sfd, &game_info, sem_id, shared_info)) {
        close(sfd);
        cleanup(&game_info);
        remove_semaphore(sem_id);
        exit(EXIT_FAILURE);
      }

      close(sfd);
      remove_semaphore(sem_id);
      cleanup(&game_info);

      break;

    default:
      // Thinker
      if (close(game_info.pipe_read_end) == -1) {
        perror("failed to close read end of pipe\n");
        exit(EXIT_FAILURE);
      }

      sigset_t sigset;
      int sig;
      if (-1 == sigemptyset(&sigset)) {
        fprintf(stderr, "failed to initialize signal set.\n");
      }
      if (-1 == sigaddset(&sigset, SIGUSR1)) {
        fprintf(stderr, "failed to add signal USR1 to set.\n");
      }
      if (-1 == sigaddset(&sigset, SIGCHLD)) {
        fprintf(stderr, "failed to add signal CHLD to set.\n");
      }
      sigprocmask(SIG_BLOCK, &sigset, NULL);
      while (1) {
#ifdef DEBUG
        printf("[DEBUG]: THINKER: suspending until SIGUSR1 is received\n");
#endif
        if (sigwait(&sigset, &sig)) {
          fprintf(stderr, "Failed to wait for the signal\n");
          break;
        }
        if (SIGCHLD == sig) {
#ifdef DEBUG
          fprintf(stderr, "connector process terminated, stopping thinker\n");
#endif
          break;
        }
#ifdef DEBUG
      lock(sem_id);
      print_shm_content(shared_info, "THINKER");
      unlock(sem_id);
#endif
        lock(sem_id);
        if (shared_info->board_info_ready) {
#ifdef DEBUG
          printf("[DEBUG]: THINKER: board info ready.\n");
#endif
          shared_info->board_info_ready = 0;
          unlock(sem_id);
          if (think(sem_id, shared_info)) {
            fprintf(stderr, "error in thinker process\n");
            break;
          }
        } else {
#ifdef DEBUG
          printf(
              "[DEBUG]: THINKER: received SIGUSR1 but board info not ready.\n");
#endif
          unlock(sem_id);
        }
      }

      wait(NULL);
      if (cleanup(&game_info)) {
        fprintf(stderr, "cleanup failed\n");
        exit(EXIT_FAILURE);
      }

      if (shared_info->board_info_created) {
        mark_segment_for_deletion(shared_info->board_shm_id);
      }
      mark_segment_for_deletion(shm_id);
  }

  exit(EXIT_SUCCESS);
}
