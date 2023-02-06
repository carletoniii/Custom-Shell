#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int main(){
  // Reading a line of input
  char *line = NULL;
  size_t n = 0;
  for (;;) {
    ssize_t line_length = getline(&line, &n, stdin); 
  }
}
