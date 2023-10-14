#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
  if (argc != 4) {
    printf("Correct usage: %s <IP address> <port number> <file_path>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char* host = argv[1];
  int port = atoi(argv[2]);
  int sock = 0;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE] = { 0 };

  // Create socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation error");
    return -1;
  }

  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, host, &server_addr.sin_addr);
  server_addr.sin_port = htons(port);

  // Connect to the server
  if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Connection Failed");
    return -1;
  }

  // Send the file name
  char* file_name = argv[3];
  if (send(sock, file_name, strlen(file_name), 0) == -1) {
    perror("File name send failed");
    return -1;
  }

  FILE* file = fopen(file_name, "r");
  if (file == NULL) {
    perror("File opening failed");
    return -1;
  }

  // Send the file content
  int bytesRead;
  while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
    if (send(sock, buffer, bytesRead, 0) == -1) {
      perror("File send failed");
      fclose(file);
      return -1;
    }
  }

  fclose(file);
  printf("File sent successfully.\n");

  close(sock);
  return 0;
}
