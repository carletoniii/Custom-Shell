#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>


int main(){

  // Line pointer declaration
  char *line = NULL;
  size_t n = 0;

  // Infinite loop
  start: for (;;) {
   
    // The prompt
    printf("%s", getenv("PS1"));

    // Reading a line of input
    errno = 0;
    ssize_t line_length = getline(&line, &n, stdin); 
    if (line_length == -1 || errno != 0) {
      fprintf(stderr, "Error reading input.");
      goto start;
    }
  }
}
