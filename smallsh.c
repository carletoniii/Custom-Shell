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
#include <ctype.h>
#include <stdint.h>
#include <fcntl.h>


// String search and replace function
// Function provided by Ryan Gamford in string search and replace tutorial
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
  int childStatus;
  int bgChild;
  char *bgPID = "";
  char *fgPID = "0";
  int bgFlag = 0;
  char *infile;
  char *outfile;
  int sourceFD;
  int targetFD;
  int result;
  
  // Word pointer declarations
  char *words[512] = { NULL };
  char *temp = NULL;

  // Infinite loop
  for (;;) {
    
    start: 
    
    bgFlag = 0;
    infile = NULL;
    outfile = NULL;

    for (int n = 0; n < 512; n++) {
      words[n] = NULL;
    }

    // Input
    // Managing background processes
    while ((bgChild = waitpid(0, &childStatus, WUNTRACED | WNOHANG)) > 0) {
      printf("Child pid = %jd", (intmax_t)bgChild);
    }

    // The prompt
    char *PS1;
    if ((PS1 = getenv("PS1")) == NULL) {
      PS1 = "";
    }
    fprintf(stderr, "%s", PS1);

    // Reading a line of input
    errno = 0;
    ssize_t line_length = getline(&line, &n, stdin);

    // Error handling for getline
    if (line_length == -1 || errno != 0) {
      fprintf(stderr, "Error reading input.\n");
      goto start;
    }

    // Word splitting
    if (getenv("IFS") == NULL) {
      setenv("IFS", " \t\n", 1);
    }
    char *IFS = (getenv("IFS"));

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
      if (words[j] != NULL) {
        if (strncmp(words[j], "~/", 2) == 0) {
          str_gsub(&words[j], "~", getenv("HOME"));
        }
        str_gsub(&words[j], "$$", PID);
        str_gsub(&words[j], "$?", fgPID);
        str_gsub(&words[j], "$!", bgPID);
      }
      i--;
      j++;
    }

    // Parsing
    int y = 0;
    while (j > 0) {
      if (strcmp(words[j - 1], "#") == 0) {
        y = j - 2;
        words[j - 1] = NULL;
        i--;
      }
      j--;
      i++;
    }
    if (y > 0) {
      if (strcmp(words[y], "&") == 0) {
        words[y] = NULL;
        bgFlag = 1;
        y--;
      }
      if (y > 2) {
        if (strcmp(words[y - 1], "<") == 0) {
          infile = words[y];
          words[y] = NULL;
          words[y - 1] = NULL;
          if (y > 4) {
            if (strcmp(words[y - 3], ">") == 0) {
              outfile = words[y - 2];
              words[y - 2] = NULL;
              words[y - 3] = NULL;
            }
          }
        } else if (strcmp(words[y - 1], ">") == 0) {
          outfile = words[y];
          words[y] = NULL;
          words[y - 1] = NULL;
          if (y > 4) {
            if (strcmp(words[y - 3], "<") == 0) {
              infile = words[y - 2];
              words[y - 2] = NULL;
              words[y - 3] = NULL;
            }
          }
        }
      }
    } else {
      if (i > 0) {
        if (strcmp(words[i - 1], "&") == 0) {
          words[i - 1] = NULL;
          bgFlag = 1;
          i--;
        }
        if (i > 3) {
          if (strcmp(words[i - 2], "<") == 0) {
            infile = words[i - 1];
            words[i - 1] = NULL;
            words[i - 2] = NULL;
            if (i > 5) {
              if (strcmp(words[i - 4], ">") == 0) {
                outfile = words[i - 3];
                words[i - 3] = NULL;
                words[i - 4] = NULL;
              }
            }
          } else if (strcmp(words[i - 2], ">") == 0) {
            outfile = words[i - 1];
            words[i - 1] = NULL;
            words[i - 2] = NULL;
            if (i > 5) {
              if (strcmp(words[i - 4], "<") == 0) {
                infile = words[i - 3];
                words[i - 3] = NULL;
                words[i - 4] = NULL;
              }
            }
          }
        } 
      }
    }

    // Execution
    // Built-in commands
    if (words[0] != NULL) {
      // exit command
      if (strcmp(words[0], "exit") == 0) {
        if (words[2] != NULL) {
          fprintf(stderr, "Too many arguments.\n");
          goto start;
        }
        if (words[1] != NULL) {
          if (!isdigit(*words[1])) {
            fprintf(stderr, "Argument is not an integer.\n");
            goto start;
          }
        }
        if (words[1] == NULL) {
          fprintf(stderr, "\nexit\n");
          exit((int)*fgPID);
        }
        fprintf(stderr, "\nexit\n");
        exit(atoi(words[1]));        
      }

      // cd command
      if (strcmp(words[0], "cd") == 0) {
        if (words[2] != NULL) {
          fprintf(stderr, "Too many arguments.\n");
          goto start;
        }
        if (words[1] == NULL) {
          words[1] = getenv("HOME");
          strcat(words[1], "/");
        }
        chdir(words[1]);
        goto start;
      }
    }

    // Non-built-in commands

    pid_t spawnPid = fork();

    switch(spawnPid) {
    case -1:
      perror("fork()\n");
      exit(1);
      break;
    case 0:
      // printf("CHILD(%d) running in command\n", getpid());
      if (infile != NULL) {
        sourceFD = open(infile, O_RDONLY);
        if (sourceFD == -1) {
          perror("source open()");
          exit(1);
        } 
        result = dup2(sourceFD, 0);
        if (result == -1) {
          perror("source dup2()");
          exit(2);
        }
      }
      if (outfile != NULL) {
        targetFD = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (targetFD == -1) {
          perror("target open()");
          exit(1);
        }
        result = dup2(targetFD, 1);
        if (result == -1) {
          perror("target dup2()");
          exit(2);
        }
      }
      execvp(words[0], words);
      // perror("execvp");
      if (infile != NULL) {
        close(sourceFD);
      }
      if (outfile != NULL) {
        close(targetFD);
      }
      exit(0);
      break;
    default:
      if (bgFlag == 0) {
        spawnPid = waitpid(spawnPid, &childStatus, 0);
        fgPID = malloc(10 * sizeof(int));
        sprintf(fgPID, "%d", spawnPid);
        // printf("PARENT(%d): child(%d) terminated. Exiting\n", getpid(), spawnPid);   
      } else {
        bgPID = malloc(10 * sizeof(int));
        sprintf(bgPID, "%d", spawnPid);
      }
      goto start;
    }
  }
  return 0;
}



