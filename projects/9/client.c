#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8080
#define MAX_COMMAND_LENGTH 1024
#define MAX_OUTPUT_LENGTH 4096

int main() {
    int sock = 0;
    struct sockaddr_in server_addr;
    char command[MAX_COMMAND_LENGTH];
    char output[MAX_OUTPUT_LENGTH];

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, '0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    printf("Enter a command to send to the server: ");
    fgets(command, MAX_COMMAND_LENGTH, stdin);

    // Send the command to the server
    send(sock, command, strlen(command), 0);
    printf("Command sent to the server: %s", command);

    // Receive the output from the server
    read(sock, output, MAX_OUTPUT_LENGTH);
    printf("Output received from the server:\n%s", output);

    close(sock);
    return 0;
}
