#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

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

void* handle(void* arg) {
  int client_socket = *((int*)arg);
  int index = -1;
  char message[BUFFER_SIZE];

  for (int i = 0; i < num_clients; i++) {
    if (clients[i] == client_socket) {
      index = i;
      break;
    }
  }

  while (1) {
    int bytes_received = recv(client_socket, message, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
      // Client has disconnected
      close(client_socket);
      printf("Client %s disconnected\n", nicknames[index]);
      broadcast(index, strcat(nicknames[index], " left!\n"));
      nicknames[index] = nicknames[num_clients - 1];
      clients[index] = clients[num_clients - 1];
      num_clients--;
      pthread_exit(NULL);
    }

    message[bytes_received] = '\0';
    broadcast(index, message);
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
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, host, &server_addr.sin_addr);

  if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("Socket binding failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_socket, MAX_CLIENTS) == -1) {
    perror("Socket listening failed");
    exit(EXIT_FAILURE);
  }

  printf("-----Server UP-----\n");

  struct sockaddr_in client_addr;
  int client_socket;
  pthread_t thread_id;

  while (1) {
    socklen_t client_addr_len = sizeof(client_addr);
    if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len)) == -1) {
      perror("Client acceptance failed");
      exit(EXIT_FAILURE);
    }

    printf("Connected with %s\n", inet_ntoa(client_addr.sin_addr));

    char nickname[BUFFER_SIZE] = "NICK";
    send(client_socket, nickname, strlen(nickname), 0);
    char client_nickname[BUFFER_SIZE];
    bzero(client_nickname, BUFFER_SIZE);
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

    if (pthread_create(&thread_id, NULL, handle, (void*)&client_socket) < 0) {
      perror("Could not create thread");
      return EXIT_FAILURE;
    }
  }

  close(server_socket);
  return EXIT_SUCCESS;
}
