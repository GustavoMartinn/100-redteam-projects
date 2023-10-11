#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h> 


#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int clients[MAX_CLIENTS];
char* nicknames[MAX_CLIENTS];
int num_clients = 0;

void broadcast(int sender, const char* message) {
  for (int i = 0; i < num_clients; i++) {
    if (i != sender) {
      send(clients[i], message, strlen(message), 0);
    }
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

  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, host, &server_addr.sin_addr);
  server_addr.sin_port = htons(port);

  if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("Socket binding failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_socket, MAX_CLIENTS) == -1) {
    perror("Socket listening failed");
    exit(EXIT_FAILURE);
  }

  printf("-----Server UP-----\n");

  fd_set read_fds, temp_fds;
  int max_fd = server_socket;

  FD_ZERO(&read_fds);
  FD_SET(server_socket, &read_fds);

  while (1) {
    temp_fds = read_fds;

    if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) == -1) {
      perror("Select failed");
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i <= max_fd; i++) {
      if (FD_ISSET(i, &temp_fds)) {
        if (i == server_socket) {
          // New connection
          struct sockaddr_in client_addr;
          socklen_t client_addr_len = sizeof(client_addr);
          int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

          if (client_socket == -1) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
          }

          printf("Connected with %s\n", inet_ntoa(client_addr.sin_addr));

          char nickname[BUFFER_SIZE] = "NICK";
          send(client_socket, nickname, strlen(nickname), 0);
          char client_nickname[BUFFER_SIZE];
          recv(client_socket, client_nickname, BUFFER_SIZE, 0);

          nicknames[num_clients] = strdup(client_nickname);
          clients[num_clients] = client_socket;
          num_clients++;

          printf("Nickname is %s\n", client_nickname);

          char welcome_message[BUFFER_SIZE] = "";
          strcat(welcome_message, client_nickname);
          strcat(welcome_message, " joined!\n");
          broadcast(num_clients - 1, welcome_message);

          char connection_message[] = "Connected to server!\n";
          send(client_socket, connection_message, strlen(connection_message), 0);

          FD_SET(client_socket, &read_fds);
          if (client_socket > max_fd)
            max_fd = client_socket;
        }
        else {
          // Handle client messages
          char message[BUFFER_SIZE];
          int bytes_received = recv(i, message, BUFFER_SIZE, 0);
          if (bytes_received <= 0) {
            // Client disconnected
            close(i);
            FD_CLR(i, &read_fds);

            int index = -1;
            for (int j = 0; j < num_clients; j++) {
              if (clients[j] == i) {
                index = j;
                break;
              }
            }

            if (index != -1) {
              printf("Client %s disconnected\n", nicknames[index]);
              char disconnect_message[BUFFER_SIZE];
              snprintf(disconnect_message, BUFFER_SIZE, "%s left!\n", nicknames[index]);
              broadcast(index, disconnect_message);

              free(nicknames[index]);
              for (int j = index; j < num_clients - 1; j++) {
                clients[j] = clients[j + 1];
                nicknames[j] = nicknames[j + 1];
              }
              num_clients--;
            }
          }
          else {
            message[bytes_received] = '\0';
            int index = -1;
            for (int j = 0; j < num_clients; j++) {
              if (clients[j] == i) {
                index = j;
                break;
              }
            }
            broadcast(index, message);
          }
        }
      }
    }
  }

  close(server_socket);
  return EXIT_SUCCESS;
}
