#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <string.h>


int main(){

  // Line pointer declaration
  char *line = NULL;
  size_t n = 0;
  
  // Word pointer declarations
  char *words[512];

  // Infinite loop
  start: for (;;) {
   
    // The prompt
    fprintf(stderr, "%s", getenv("PS1"));

    // Reading a line of input
    errno = 0;
    ssize_t line_length = getline(&line, &n, stdin);

    // Error handling for getline
    if (line_length == -1 || errno != 0) {
      fprintf(stderr, "Error reading input.");
      goto start;
    }
  
    // Word splitting
    char *IFS = " \t\n";
    
    int i = 0;
    words[i] = strdup(strtok(line,IFS));
    while (words[i] != NULL) {
      i++;
      words[i] = strtok(NULL, IFS);
    }
    
  }
}
