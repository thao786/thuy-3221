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
// array: ------begin------end--------
struct Job {
	int thread;
	int created_at;
} job;

float lambda, mu;
int server_count, client_count, done_count; // how many threads are done wt their jobs?
int begin, end, length, program_done;
struct Job* queue;
pthread_t *servers, *clients, clock_thread;
pthread_mutex_t queue_mutex, all_done_mutex, tick_mutex;
pthread_cond_t all_done, tick;


void* client(void *t) {
	long id = (long)t;
	printf("client created");
	// enqueue
	while (program_done < 1) {
		pthread_mutex_lock(&tick_mutex);
		pthread_cond_wait(&tick, &tick_mutex);
			printf("tick signal received %ld \n", id);
			done_count = done_count + 1;
			pthread_cond_signal(&all_done);
		pthread_mutex_unlock(&tick_mutex);

		// pthread_mutex_lock(&queue_mutex);
		// 	printf("enqueue");
		// 	queue[end].created_at = (unsigned)time(NULL);
		// 	end = end + 1;
		// 	done_count = done_count + 1;
		// 	pthread_cond_signal(&all_done);
		// pthread_mutex_unlock(&queue_mutex);

	}

	pthread_exit(NULL);
}

void* server() {printf("server created");
	// dequeue
	while (program_done < 1) {
		pthread_mutex_lock(&tick_mutex);
		pthread_cond_wait(&tick, &tick_mutex);
		pthread_mutex_unlock(&tick_mutex);

		pthread_mutex_lock(&queue_mutex);
			printf("dequeue");
			if (length > 0) {
				length = length - 1;
				begin = begin + 1; // move begin 1 spot to the right
			}
			done_count = done_count + 1;
			pthread_cond_signal(&all_done);
		pthread_mutex_unlock(&queue_mutex);
	}
}

void* clock_fn() {
	printf("clock created");
	pthread_cond_broadcast(&tick);

	int i;
	for( i = 0; i < 10; i = i + 1 ) {
		pthread_mutex_lock(&all_done_mutex);
			// while (done_count < client_count + server_count) {
			while (done_count < client_count) {
				pthread_cond_wait(&all_done, &all_done_mutex);
				printf("clock: signal received %d \n", done_count);
			}

			done_count = 0;
			pthread_cond_broadcast(&tick);
			printf("clock tick");
		pthread_mutex_unlock(&all_done_mutex);
	}
	program_done = 1;
	printf("clock exits \n");
	pthread_exit(NULL);
}

int main(int argc, char **argv) {
	server_count = 2, client_count = 1;
	servers = (pthread_t*)malloc(server_count*sizeof(pthread_t));
	clients = (pthread_t*)malloc(client_count*sizeof(pthread_t));
	queue = malloc(1000*sizeof(job));
	begin = 0, end = 0, length = 0,
	done_count = 0, program_done = 0;
	lambda = 0.005, mu = 0.01;

	pthread_mutex_init(&queue_mutex, NULL);
	pthread_mutex_init(&all_done_mutex, NULL);
	pthread_mutex_init(&tick_mutex, NULL);

	pthread_cond_init(&all_done, NULL);
	pthread_cond_init(&tick, NULL);


	// create clients
	int i;
	for( i = 0; i < client_count; i = i + 1 ) {
		int err = pthread_create(&(clients[i]), NULL, client, (void *)(clients[i]));
		pthread_join(clients[i], NULL);
		printf("     %ld \n", clients[i]);
	}

	// // create servers
	// for( i = 0; i < server_count; i = i + 1 ) {
	// 	pthread_create(&(servers[i]), NULL, &server, NULL);
	// 	pthread_join(servers[i], NULL);
	// }

	// create clock thread
	pthread_create(&clock_thread, NULL, clock_fn, NULL);
	pthread_join(clock_thread, NULL);

	return 0;
}
