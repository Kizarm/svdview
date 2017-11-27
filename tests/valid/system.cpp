#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void Logging (const char * buff) {
}

/// replace std system() call - redirect stdout, stderr to user fnc.
int RunCmd (const char * cmd) {
  int pipefd[2], result = -1, status = -1;
  result = pipe (pipefd);
  if (result) return result;

  pid_t cpid = fork();
  if (cpid < 0) return -1;
  if (cpid == 0) {   // child
    close (pipefd[0]);    // close reading end in the child
    dup2  (pipefd[1], 1); // send stdout to the pipe
    dup2  (pipefd[1], 2); // send stderr to the pipe
    result = execl ("/bin/sh", "sh", "-c", cmd, (char *) 0);
    close (pipefd[1]);    // this descriptor is no longer needed
  } else {           // parent
    const unsigned max = 1024;
    char buffer [max + 4];
    close (pipefd[1]); // close the write end of the pipe in the parent
    for (;;) {
      int n = read (pipefd[0], buffer, max);
      if (n <= 0) break;
      buffer[n] = '\0';
      Logging (buffer);
    }
    if (waitpid(cpid, &status, 0) == -1) {
      // handle error
    }
    close (pipefd[0]);
  }
  return status;
}
