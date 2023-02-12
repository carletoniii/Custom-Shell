#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>


// String search and replace function
char *str_gsub(char *restrict *restrict haystack, char const *restrict needle, char const *restrict sub)
{
  char *str = *haystack;
  size_t haystack_len = strlen(str);
  size_t const needle_len = strlen(needle),
               sub_len = strlen(sub);

  for (; (str = strstr(str, needle));) {
    ptrdiff_t off = str - *haystack;
    if (sub_len > needle_len) {
      str = realloc(*haystack, sizeof **haystack * (haystack_len + sub_len - needle_len + 1));
      if (!str) goto exit;
      *haystack = str;
      str = *haystack + off;
    }
    memmove(str + sub_len, str + needle_len, haystack_len + 1 - off - needle_len);
    memcpy(str, sub, sub_len);
    haystack_len = haystack_len + sub_len - needle_len;
    str += sub_len;
  }
  str = *haystack;
  if (sub_len < needle_len) {
    str = realloc(*haystack, sizeof **haystack * (haystack_len + 1));
    if (!str) goto exit;
    *haystack = str;
  }

exit:
  return str;
}

int main(void){

  // Line pointer declaration
  char *line = NULL;
  size_t n = 0;
  
  // Word pointer declarations
  char *words[512];
  char *temp = NULL;

  // Infinite loop
  for (;;) {
    
    start: 

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
    if (getenv("IFS") == NULL) {
      setenv("IFS", " \t\n", 1);
    }
    char *IFS = getenv("IFS");
    
    int i = 0;
    temp = strtok(line, IFS);
    while (temp != NULL) {
      words[i] = strdup(temp);
      i++;
      temp = strtok(NULL, IFS);
    }
    
    // Expansion
    int j = 0;
    char *PID = malloc(sizeof(int) * 8);
    sprintf(PID, "%d", getpid());
    while (i > 0) {
      if (strncmp(words[j], "~/", 2) == 0) {
        str_gsub(&words[j], "~", getenv("HOME"));
      }
      str_gsub(&words[j], "$$", PID); 
      printf("%s\n", words[j]);
      i--;
      j++;
    } 
    
    // Parsing
    
    // Execution
    char *newargv[] = { "/bin/ls", "-al", NULL };
    int childStatus;

    pid_t spawnPid = fork();

    switch(spawnPid) {
    case -1:
      perror("fork()\n");
      exit(1);
      break;
    case 0:
      printf("CHILD(%d) running in command\n", getpid());
      execvp(newargv[0], newargv);
      perror("execvp");
      exit(2);
      break;
    default:
      spawnPid = waitpid(spawnPid, &childStatus, 0);
      printf("PARENT(%d): child(%d) terminated. Exiting\n", getpid(), spawnPid);
      exit(0);
      break;
    }
  }
  return 0;
}



