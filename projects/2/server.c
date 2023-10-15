#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int num_clients = 0;

void broadcast(int server_socket, struct sockaddr_in* clients, int num_clients, const char* message, socklen_t addr_len) {
  for (int i = 0; i < num_clients; i++) {
    sendto(server_socket, message, strlen(message), 0,
      (struct sockaddr*)&clients[i], addr_len);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Correct usage: %s <IP address> <port number>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char* host = argv[1];
  int port = atoi(argv[2]);

  int server_socket;
  struct sockaddr_in server_addr;

  if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, host, &server_addr.sin_addr);
  server_addr.sin_port = htons(port);

  if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("Socket binding failed");
    exit(EXIT_FAILURE);
  }

  printf("-----Server UP-----\n");

  struct sockaddr_in clients[MAX_CLIENTS];
  char buffer[BUFFER_SIZE];

  while (1) {
    memset(buffer, 0, BUFFER_SIZE);
    socklen_t addr_len = sizeof(struct sockaddr_in);

    int bytes_received = recvfrom(server_socket, buffer, BUFFER_SIZE, 0,
      (struct sockaddr*)&clients[num_clients], &addr_len);

    if (bytes_received == -1) {
      perror("Receive failed");
      exit(EXIT_FAILURE);
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clients[num_clients].sin_addr), client_ip, INET_ADDRSTRLEN);

    printf("Received message from %s:%d - %s", client_ip, ntohs(clients[num_clients].sin_port), buffer);

    for (int i = 0; i < num_clients; i++) {
      if (clients[i].sin_addr.s_addr == clients[num_clients].sin_addr.s_addr &&
        clients[i].sin_port == clients[num_clients].sin_port) {
        num_clients--;
        break;
      }
    }

    num_clients++;

    // Broadcast the received message to all clients
    broadcast(server_socket, clients, num_clients, buffer, addr_len);
  }

  close(server_socket);
  return EXIT_SUCCESS;
}
