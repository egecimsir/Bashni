#ifndef NETWORK_H_
#define NETWORK_H_ "header guard"

#define BUFSIZE 1024

int create_tcp_socket(char *host_name, uint16_t port_number);

int read_next_message(int socket_fd, char buf[BUFSIZE]);

#endif  // NETWORK_H_
