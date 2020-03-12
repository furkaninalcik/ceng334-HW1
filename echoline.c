/* echoline.c - Echo to stdout each line read from stdin

https://cis.temple.edu/~ingargio//cis307/readings/echoline.c

 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define LINEMAX 256

int 
main()
{
  char line[LINEMAX];
  int n;

  for ( ; ; ) {
    /* on the socket pair we cannot use printf, scanf, etc. */
    /* For printf, scanf, etc. we need to have a pseudoterminal. */ 
    n = read(STDIN_FILENO, line, LINEMAX-1);
    if (n < 0) break;
    line[n] = '\0';
    write(STDOUT_FILENO, line, n);
  }
  return 0;
}