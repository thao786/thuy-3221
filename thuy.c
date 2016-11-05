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

// add element to end: end is the index of free spot
// remove element from begin: begin is the index of first occupied spot
// array: -----begin------end--------
struct Job {
	int thread;
	int created_at;
} job;

float lambda, mu;
int server_count, client_count;
int begin, end, length;
struct Job queue[1000];
pthread_t *servers, *clients, clock_thread;

void* client() {
	printf("enqueue");

	// enqueue
	queue[end].created_at = (unsigned)time(NULL);
	end = end + 1;

}

void* server() {
	printf("dequeue");

	// dequeue
	if (length > 0) {
		length = length - 1;
		begin = begin + 1;
	}
}

void* clock_fn() {
	printf("clock tick");
}

int main(int argc, char **argv)
{
	// printf("%s: syst time for children: %d microseconds\n",argv[0],time);
	server_count = 2, client_count = 2;
	servers = malloc(server_count*sizeof(pthread_t));
	clients = malloc(client_count*sizeof(pthread_t));
	begin = 0, end = 0, length = 0;
	lambda = 0.005, mu = 0.01;

	// create clients
	int i;
	for( i = 0; i < client_count; i = i + 1 ) {
		pthread_create(&(clients[i]), NULL, &client, NULL);
		pthread_join(clients[i], NULL); 
	}

	// create servers
	for( i = 0; i < server_count; i = i + 1 ) {
		pthread_create(&(servers[i]), NULL, &servers, NULL);
		pthread_join(servers[i], NULL); 
	}

	// create clock thread
	// pthread_create(&clock_thread, NULL, &clock_fn, NULL);

	return 0;
}

