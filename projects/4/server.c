#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Correct usage: %s <IP address> <port number>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char* host = argv[1];
  int port = atoi(argv[2]);
  int server_fd, new_socket;
  struct sockaddr_in server_addr;
  int opt = 1;
  int addrlen = sizeof(server_addr);
  char buffer[BUFFER_SIZE] = { 0 };

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Set socket options
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("Setsockopt failed");
    exit(EXIT_FAILURE);
  }

  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, host, &server_addr.sin_addr);
  server_addr.sin_port = htons(port);

  // Bind the socket
  if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(server_fd, 3) < 0) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d\n", port);

  while (1) {
    if ((new_socket = accept(server_fd, (struct sockaddr*)&server_addr, (socklen_t*)&addrlen)) < 0) {
      perror("Accept failed");
      exit(EXIT_FAILURE);
    }

    // Receive the file name
    char file_name[BUFFER_SIZE];
    int bytes_received_file_name = recv(new_socket, file_name, sizeof(file_name) - 1, 0);
    if (bytes_received_file_name == -1) {
      perror("File name receive failed");
      close(new_socket);
      continue;
    }
    file_name[bytes_received_file_name] = '\0';  // Null-terminate the received file name

    // Extract the original filename and extension
    char* original_filename = strtok(file_name, ".");
    char* extension = strtok(NULL, ".");

    // Get the current time
    time_t raw_time;
    struct tm* time_info;
    char actual_time[20];
    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(actual_time, sizeof(actual_time), "%Y%m%d%H%M%S", time_info);

    // Create the new filename with actual time and original extension
    char new_file_name[BUFFER_SIZE];
    sprintf(new_file_name, "data/%s_%s.%s", original_filename, actual_time, extension);

    printf("Received file name: %s\n", file_name);

    FILE* file = fopen(new_file_name, "w");
    if (file == NULL) {
      perror("File opening failed");
      close(new_socket);
      continue;
    }

    int total_bytes_received = 0;
    int bytes_received = 0;
    while ((bytes_received = recv(new_socket, buffer, BUFFER_SIZE, 0)) > 0) {
      total_bytes_received += bytes_received;
      fwrite(buffer, 1, bytes_received, file);
    }

    if (bytes_received == -1) {
      perror("File receive failed");
    }

    printf("Total bytes received: %d\n", total_bytes_received);

    if (fclose(file) != 0) {
      perror("File closure failed");
    }

    close(new_socket);
  }

  close(server_fd);

  return 0;
}
