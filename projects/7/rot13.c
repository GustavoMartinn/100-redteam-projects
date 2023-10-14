
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* encrypt(char* message) {
  int length = strlen(message);
  char* encrypted = malloc(length * sizeof(char));
  int step = 13;

  for (int i = 0; i < length; i++) {
    char c = message[i];
    if (c >= 'a' && c <= 'z') {
      encrypted[i] = 'a' + ((c - 'a' + step) % 26);
    }
    else if (c >= 'A' && c <= 'Z') {
      encrypted[i] = 'A' + ((c - 'A' + step) % 26);
    }
    else {
      encrypted[i] = message[i];
    }
  }

  return encrypted;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Correct usage: %s <message>\n", argv[0]);
  }
  char* message = argv[1];

  printf("Message: %s\n", message);

  char* encrypted = encrypt(message);
  printf("Encrypted: %s\n", encrypted);
}