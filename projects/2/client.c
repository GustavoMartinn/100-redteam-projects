#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

char nickname[BUFFER_SIZE];
char* host;
int port;

void* receive(void* arg) {
  int client_socket = *((int*)arg);

  while (1) {
    char message[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    int bytes_received = recvfrom(client_socket, message, BUFFER_SIZE, 0,
      (struct sockaddr*)&server_addr, &addr_len);
    if (bytes_received == -1) {
      perror("Receive failed");
      exit(EXIT_FAILURE);
    }

    message[bytes_received] = '\0';
    if (strncmp(message, nickname, strlen(nickname)) != 0)
      printf("%s\n", message);
  }
}

void* write_to_server(void* arg) {
  int client_socket = *((int*)arg);

  while (1) {
    char message[BUFFER_SIZE];
    fgets(message, BUFFER_SIZE, stdin);
    char* formatted_message = malloc(BUFFER_SIZE);
    strcpy(formatted_message, nickname);
    strcat(formatted_message, ": ");
    strcat(formatted_message, message);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); 
    inet_pton(AF_INET, host, &server_addr.sin_addr);

    sendto(client_socket, formatted_message, strlen(formatted_message), 0,
      (struct sockaddr*)&server_addr, sizeof(server_addr));
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Correct usage: %s <IP address> <port number>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  host = argv[1];
  port = atoi(argv[2]);

  int client_socket;
  struct sockaddr_in server_addr;

  if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, host, &server_addr.sin_addr);

  printf("Connected to server\n");

  printf("Choose your nickname: ");
  fgets(nickname, BUFFER_SIZE, stdin);
  nickname[strcspn(nickname, "\n")] = '\0';

  pthread_t receive_thread, write_thread;

  if (pthread_create(&receive_thread, NULL, receive, (void*)&client_socket) < 0 ||
    pthread_create(&write_thread, NULL, write_to_server, (void*)&client_socket) < 0) {
    perror("Could not create threads");
    return EXIT_FAILURE;
  }

  pthread_join(receive_thread, NULL);
  pthread_join(write_thread, NULL);

  close(client_socket);

  return EXIT_SUCCESS;
}
