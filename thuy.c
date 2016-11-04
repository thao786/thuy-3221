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
#include <pthread.h>

struct Job {
	int thread;
	int   created_at;
	struct Job *next;
};

void* client() {
	printf("syst time for children:  microseconds\n");

	// enqueue
}

int main(int argc, char **argv)
{
	// printf("%s: syst time for children: %d microseconds\n",argv[0],time);
	int server_count = 2, client_count = 2;
	pthread_t servers[server_count], clients[client_count];
	float lambda = 0.005, mu = 0.01;
	struct Job head, tail;

	// create clients
	int i;
	for( i = 0; i < client_count; i = i + 1 ) {
		int err = pthread_create(&(clients[i]), NULL, &client, NULL);
	}
}

