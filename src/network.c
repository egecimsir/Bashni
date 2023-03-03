#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>

#include "./network.h"

int create_tcp_socket(char *host_name, u_int16_t port_number) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int ret, s;
  char *port;
  if (NULL == (port = malloc(6 * sizeof(char)))) {
    fprintf(stderr, "could not allocate memory\n");
    return -1;
  }

  snprintf(port, 6 * sizeof(char), "%d", port_number);

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  s = getaddrinfo(host_name, port, &hints, &result);
  free(port);

  if (s != 0) {
      fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(s));
      return -1;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
      ret = socket(rp->ai_family, rp->ai_socktype,
                   rp->ai_protocol);
      if (ret == -1) {
          continue;
      }

      if (connect(ret, rp->ai_addr, rp->ai_addrlen) != -1) {
          break;
      }

      close(ret);
  }

  if (rp == NULL) {
      return -1;
  }

  freeaddrinfo(result);

  return ret;
}

// Read input from the TCP socket file descriptor "socket_fd"
// into "buf".
// The buffer is expected to be filled only with zeroes.
int read_next_message(int socket_fd, char buf[BUFSIZE]) {
  int len;

  for (len = 0; len < BUFSIZE-2; len++) {
    if (0 == recv(socket_fd, buf+len, 1, 0)) {
#ifdef DEBUG
      printf("[DEBUG]: nothing to read ?\n");
#endif
      break;
    }
    if ('\n' == buf[len]) {
      break;
    }
  }

  return len;
}
