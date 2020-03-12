/* socketpair.c - Example of using socketpairs.
 *       A socketpair is like a pipe, in the sense that we use
 *       an "int fd[2]" pair of descriptors. But now we can read and write
 *       at each end of the pipe, fd[0] and at fd[1].
 *       In this example we create a socketpair and then we fork.
 *       In the child we redirect standard input and output
 *       to the socketpair [1] and then we echo what we read.
 *       In the parent we get input from terminal, send to child,
 *       read from child, and print. 

https://cis.temple.edu/~giorgio/old/cis307f01/readings/socketpair.c
 
 */

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include<sys/socket.h>
#include<sys/wait.h> 
#include <stdio.h>
#include <unistd.h>
#include<stdlib.h> 
#include<fcntl.h> 
#define LINEMAX 256  /* Size of largest line we read from terminal */

int
main ()
{
  char line[LINEMAX];
  char line1[LINEMAX];
  pid_t child;
  int fd[2];
  int rc;

  rc = socketpair( AF_UNIX, SOCK_STREAM, 0, fd );
  if ( rc < 0 ) {
    perror("Cannot open socketpair");
    exit(0);
  } 
  child = fork();
  if (child < 0) {
    perror("Cannot fork");
    exit(0);
  }
  if (child == 0) { /* child - it uses fd[1] */
    close (fd[0]);
    if (fd[1] != STDIN_FILENO) { /*Redirect standard input to socketpair*/
      if (dup2(fd[1], STDIN_FILENO) != STDIN_FILENO) {
      	perror("Cannot dup2 stdin");
      	exit(0);
      }
    }
    if (fd[1] != STDOUT_FILENO) { /*Redirect standard output to socketpair*/
      if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO) {
      	perror("Cannot dup2 stdout");
      	exit(0);
      }
    }
    if (execl("./echoline", "echoline", NULL) < 0) {
      perror("Cannot exec");
      exit(0);
    }
    exit(0);
  }
  /* Parent - use to talk to child fd[0] */
  for ( ; ; ) {
    printf("Enter a line: ");
    fgets(line, LINEMAX-1, stdin);
    /* on the socket pair we cannot use printf, scanf, etc. */
    /* For printf, scanf, etc. we need to have a pseudoterminal. */ 
    write(fd[0], line, strlen(line));
    read(fd[0], line1, LINEMAX-1);
    line[strlen(line1)-1] = '\0';
    fputs(line1, stdout);
  }
  return 0;
}