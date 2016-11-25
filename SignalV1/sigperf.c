/* Author: Minas Spetsakis                            */
/* Date:   Oct 13 2016                                */
/* Signals assignement, Version 1                     */

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>


int main(int argc, char **argv)
{
  pid_t pid1, pid2, chld;
  sigset_t set;
  struct rusage usage;
  int time;

  if (argc!=1)
    {
      fprintf(stderr,"%s: No arguments needed\n", argv[0]);
      exit(1);
    }
  if (sigemptyset(&set)<0)
    {
      fprintf(stderr,"%s: Failed to create signal set: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  if (sigaddset(&set, SIGUSR1)<0)
    {
      fprintf(stderr,"%s: Failed to modify set: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  if (sigprocmask(SIG_BLOCK,&set,NULL)<0)
    {
      fprintf(stderr,"%s: Failed to mask SIG_USR1: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  pid1 = fork();
  if (pid1<0)
    {
      fprintf(stderr,"%s: Failed to fork first child: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  if (pid1==0)
    {				/* Cild executes now */
      execl("sigshooter","sigshooter","0",(char *)NULL);
      fprintf(stderr,"%s: Failed to exec first child: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  pid2 = fork();
  if (pid2<0)
    {
      fprintf(stderr,"%s: Failed to fork second child: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  if (pid2==0)
    {				/* Cild executes now */
      char pidstr[32];
      snprintf(pidstr,31,"%d",pid1);
      execl("sigshooter","sigshooter",pidstr,(char *)NULL);
      fprintf(stderr,"%s: Failed to exec second child: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  chld = wait(NULL);
  if (chld==pid1)
    printf("first sigshooter terminated\n");
  else
    printf("second sigshooter terminated\n");
  chld = wait(NULL);
  if (chld==pid1)
    printf("first sigshooter terminated\n");
  else
    printf("second sigshooter terminated\n");
  if (getrusage(RUSAGE_SELF,&usage)<0)
    {
      fprintf(stderr,"%s: Failed to retrieve times for self: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  time = usage.ru_utime.tv_sec*1000000 + usage.ru_utime.tv_usec;
  printf("%s: user time for self    : %d microseconds\n",argv[0],time);
  time = usage.ru_stime.tv_sec*1000000 + usage.ru_utime.tv_usec;
  printf("%s: syst time for self    : %d microseconds\n",argv[0],time);
  if (getrusage(RUSAGE_CHILDREN,&usage)<0)
    {
      fprintf(stderr,"%s: Failed to retrieve times for self: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  time = usage.ru_utime.tv_sec*1000000 + usage.ru_utime.tv_usec;
  printf("%s: user time for children: %d microseconds\n",argv[0],time);
  time = usage.ru_stime.tv_sec*1000000 + usage.ru_utime.tv_usec;
  printf("%s: syst time for children: %d microseconds\n",argv[0],time);
}
