#include "connection.h"

//LINKS REFERENCIAS:
//https://www.man7.org/linux/man-pages/man2/socket.2.html
//https://man7.org/linux/man-pages/man7/socket.7.html
//https://www.man7.org/linux/man-pages/man3/setsockopt.3p.html
//https://man7.org/linux/man-pages/man2/setsockopt.2.html
//https://linux.die.net/man/3/htons
//https://linux.die.net/man/3/inet_aton
//https://www.howtogeek.com/225487/what-is-the-difference-between-127.0.0.1-and-0.0.0.0/
//https://www.man7.org/linux/man-pages/man2/bind.2.html
//https://www.man7.org/linux/man-pages/man2/accept.2.html


// Initialize server
int init_server(char* IP, int port) {

  // Server socket address
  struct sockaddr_in server_addr;

  // Create socket for server
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);

  // Socket settings
  int opt = 1;
  int ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

  // Save IP & PORT
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_aton(IP, &server_addr.sin_addr);
  server_addr.sin_port = htons(port);

  // Assign IP & PORT for server socket
  int ret2 = bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));

  // Listen for clients
  int ret3 = listen(server_socket, 1);

  return server_socket;
}
