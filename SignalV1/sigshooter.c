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

pid_t sibl;
int cnt = 0;

void USR1_action(int sig, siginfo_t *info,void *X)
{
  if (info==NULL)
    {
      fprintf(stderr,"USR1_action: improper NULL info pointer in handler: %s\n",
	      strerror(errno));
      exit(1);
    }
  sibl = info->si_pid;
  cnt++;
}

int main(int argc, char **argv)
{
  sigset_t set;
  struct sigaction act;
  int loc_sibl, i;

  if (argc!=2)
    {
      fprintf(stderr,"%s: Expected one argument, got %d\n", argv[0],argc-1);
      exit(1);
    }
  act.sa_sigaction = USR1_action;
  act.sa_flags = SA_SIGINFO;
  if (sigaction(SIGUSR1, &act, NULL)<0)
    {
      fprintf(stderr,"%s: Failed to install signal handler: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  if (sigemptyset(&set)<0)
    {
      fprintf(stderr,"%s: Failed to create empty signal mask: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  if (sigprocmask(0,(sigset_t *)NULL,&set)<0)
    {
      fprintf(stderr,"%s: Failed to retrieve signal mask: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  if (sigdelset(&set, SIGUSR1)<0){
      fprintf(stderr,"%s: Failed to change signal mask: %s\n",argv[0],
	      strerror(errno));
      exit(1);
    }
  loc_sibl = atoi(argv[1]);
  if (loc_sibl!=0)
    {
      printf("I am the second child, my PID is %d, my sibling's PID is %d\n",
	     getpid(), loc_sibl);
      sibl = loc_sibl;
      kill(sibl,SIGUSR1);
    }
  else
    printf("I am the first child, my PID is %d, my sibling's PID is %d\n",
	   getpid(), loc_sibl);
  for (i=1;i<=99;i++)
    {
      sigsuspend(&set);
      kill(sibl,SIGUSR1);
    }
  sigsuspend(&set);
  if (loc_sibl==0)
    kill(sibl,SIGUSR1);
  printf("I am %d, my sibling is %d and got %d signals\n",
	 getpid(), sibl,cnt);
}
