
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* encrypt(char* message, int step) {
  int length = strlen(message);
  char* encrypted = malloc(length * sizeof(char));

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
  if (argc != 3) {
    printf("Correct usage: %s <step> <message>\n", argv[0]);
  }

  int step = atoi(argv[1]);
  char* message = argv[2];

  printf("Step: %d\n", step);
  printf("Message: %s\n", message);

  char* encrypted = encrypt(message, step);
  printf("Encrypted: %s\n", encrypted);
}