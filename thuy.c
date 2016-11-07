#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <math.h>
#include <sys/resource.h>
#include <pthread.h>
#include <inttypes.h>


// add element to end: end is the index of free spot
// remove element from begin: begin is the index of first occupied spot
// array: ------begin------end--------
struct Job {
	int thread;
	int created_at;
} job;

float lambda, mu;
int server_count, client_count;
int begin, end, length, program_done;
int* job_status;
struct Job* queue;
pthread_t *servers, *clients, clock_thread;
pthread_mutex_t queue_mutex, all_done_mutex, tick_mutex;
pthread_cond_t all_done, tick;
int ticks;

// job statistic variables
long total_waiting_time, total_jobs_ever, 
	total_execution_time, queue_length,
	total_interarrival, previous_arrival_time;

int check_threads_done() {
	int i=0;
	while (i < client_count + server_count) {
		if (job_status[i] < 1) {
			return 0;
		}
		i = i+1;
	}
	return 1;
}

void reset_all() {
	int i;
	for(i = 0; i < client_count+server_count; i = i + 1) {
		job_status[i] = 0;
	}
}

long now() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_usec;
}

void* client(void *t) {
	// index in job status array
	int index = (int)t;
	printf("client created");

	// enqueue
	while (program_done < 1) {
		pthread_mutex_lock(&tick_mutex);
			while (job_status[index] == 1) {
				pthread_cond_wait(&tick, &tick_mutex);
			}
			job_status[index] = 0;
			// printf("client   tick signal received %d \n", index);
		pthread_mutex_unlock(&tick_mutex);

		
		pthread_mutex_lock(&queue_mutex);
			float random = (rand() + 1.0) / (RAND_MAX+2.0);
			if (random < lambda) {
				// printf("    enqueue, random = %f \n", random);
				queue[end].created_at = now();
				previous_arrival_time = now();
				end = end + 1;
				job_status[index] = 1;
				total_jobs_ever = total_jobs_ever + 1;
		
				if (total_jobs_ever > 1) {
					total_interarrival = total_interarrival + now() - previous_arrival_time;
				}
				pthread_cond_signal(&all_done);
			}
		pthread_mutex_unlock(&queue_mutex);
		
	}

	// printf("client exits \n");
	pthread_exit(NULL);
}

void* server(void *t) {
	// index in job status array
	int index = (int)t;
	// printf("server created");

	// enqueue
	while (program_done < 1) {
		pthread_mutex_lock(&tick_mutex);
			while (job_status[index] == 1) {
				pthread_cond_wait(&tick, &tick_mutex);
			}
			job_status[index] = 0;
			// printf("server  tick signal received %d \n", index);
		pthread_mutex_unlock(&tick_mutex);

		int start_execution_time = now();
		pthread_mutex_lock(&queue_mutex);
			float random = (rand() + 1.0) / (RAND_MAX+2.0);
			if (random > mu) {
				total_waiting_time = total_waiting_time + (now()-queue[begin].created_at);
				// printf("    dequeue, random = %f \n", random);
				begin = begin + 1;
				total_execution_time = total_execution_time + (now()-start_execution_time);
				job_status[index] = 1;
				pthread_cond_signal(&all_done);
			}
		pthread_mutex_unlock(&queue_mutex);
	}

	// printf("server exits \n");
	pthread_exit(NULL);
}

void* clock_fn() {
	// printf("clock created");
	pthread_cond_broadcast(&tick);

	int i;
	for( i = 0; i < ticks; i = i + 1 ) {
		queue_length = queue_length + (end - begin + 1);
		pthread_mutex_lock(&all_done_mutex);
			while (check_threads_done() == 0) {
				pthread_cond_wait(&all_done, &all_done_mutex);
				// printf("clock: signal received %d \n", check_threads_done());
			}

			reset_all();
			// printf("clock tick \n");
			pthread_cond_broadcast(&tick);			
		pthread_mutex_unlock(&all_done_mutex);
	}
	program_done = 1;
	// printf("clock exits \n");
	pthread_exit(NULL);
}

int main(int argc, char **argv) {
	server_count = 2, client_count = 2;
	servers = (pthread_t*)malloc(server_count*sizeof(pthread_t));
	clients = (pthread_t*)malloc(client_count*sizeof(pthread_t));
	job_status = (int*)malloc((client_count+server_count)*sizeof(int));
	queue = malloc(1000*sizeof(job));
	begin = 0, end = 0, length = 0, program_done = 0;
	lambda = 0.005, mu = 0.01;
	total_waiting_time=0, total_jobs_ever=0, 
	total_execution_time=0, queue_length=0;
	ticks = 10, total_interarrival = 0, previous_arrival_time = 0;

	pthread_mutex_init(&queue_mutex, NULL);
	pthread_mutex_init(&all_done_mutex, NULL);
	pthread_mutex_init(&tick_mutex, NULL);

	pthread_cond_init(&all_done, NULL);
	pthread_cond_init(&tick, NULL);

	int i = 0;
	
	reset_all();  // reset all clients and server to origin


	// create clients
	for(i = 0; i < client_count; i = i + 1) {
		int err = pthread_create(&(clients[i]), NULL, client, (void *)(i));
		if (err != 0) {
	        printf("Could not create thread. Exit.\n  ");
	        exit(0);
	    }
	}

	// create servers
	for( i = 0; i < server_count; i = i + 1 ) {
		int err = pthread_create(&(servers[i]), NULL, &server, (void *)(i + client_count));
		if (err != 0) {
	        printf("Could not create thread. Exit.\n  ");
	        exit(0);
	    }
	}

	// create clock thread
	int err = pthread_create(&clock_thread, NULL, clock_fn, NULL);
	if (err != 0) {
        printf("Could not create thread. Exit.\n  ");
        exit(0);
    }

	// join all threads
	pthread_join(clock_thread, NULL);

	for(i = 0; i < client_count; i = i + 1) {
		pthread_join(clients[i], NULL);
	}

	for(i = 0; i < server_count; i = i + 1) {
		pthread_join(servers[i], NULL);
	}

	pthread_mutex_destroy(&queue_mutex);
	pthread_mutex_destroy(&all_done_mutex);
	pthread_mutex_destroy(&tick_mutex);

	pthread_cond_destroy(&all_done);
	pthread_cond_destroy(&tick);

    
	float avg_waiting_time = total_waiting_time / total_jobs_ever;
	float avg_exe_time = total_execution_time / total_jobs_ever;
	float avg_interarrival = total_interarrival / total_jobs_ever;
	float avg_turnaround_time = (total_waiting_time + total_execution_time) / total_jobs_ever;
	float avg_queue_length = queue_length / ticks;

	printf("total jobs ever:  %d . \n  ", total_jobs_ever);
	printf("Average waiting time (AWT)  %f microseconds. \n  ", avg_waiting_time);
	printf("Average execution time (AXT)  %f microseconds. \n  ", avg_exe_time);
	printf("Average turnaround time (ATA)  %f microseconds. \n  ", avg_turnaround_time);
	printf("Average queue length (AQL)  %f. \n  ", avg_queue_length);
	printf("Average interarrivaltime (AIA)  %f microseconds. \n  ", avg_interarrival);

	return 0;
}
