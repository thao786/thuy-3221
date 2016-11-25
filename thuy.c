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
int server_count, client_count, capacity;
int begin, end, length, program_done;
int* job_status;
struct Job* queue;
pthread_t *servers, *clients, clock_thread;
pthread_mutex_t queue_mutex, all_done_mutex, tick_mutex;
pthread_cond_t all_done, tick;
int ticks, clock_time;

// job statistic variables
int total_waiting_time, total_jobs_ever, 
	total_execution_time, queue_length,
	total_interarrival, previous_arrival_time;

// check if all clients and server has done with their jobs for the current tick
// if found any undone job, return 0 right away	
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

// reset all client and server job status to 0
// get ready for the next tick
void reset_all() {
	int i;
	for(i = 0; i < client_count+server_count; i = i + 1) {
		job_status[i] = 0;
	}
}

// get the current microseconds
long now() {
	return clock_time;
}

// function to be executed by all client threads
void* client(void *t) {
	// index in job status array
	int index = (int)t;

	// keep going until clock thread stops
	while (program_done < 1) {
		// wait for clock to tick
		pthread_mutex_lock(&tick_mutex);
			while (job_status[index] == 1) {
				pthread_cond_wait(&tick, &tick_mutex);
			}
			job_status[index] = 0;
		pthread_mutex_unlock(&tick_mutex);

		// wait to access job queue
		pthread_mutex_lock(&queue_mutex);
			// generate a random between 0 and 1
			float random = (rand() + 1.0) / (RAND_MAX+2.0);
			if (random < lambda) {
				if (end >= capacity) {
					capacity = capacity + ticks;
					queue = malloc(capacity*sizeof(job));
				}
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

	pthread_exit(NULL);
}

// function to be executed by all server threads
void* server(void *t) {
	// index in job status array
	int index = (int)t;

	// keep going until clock thread stops
	while (program_done < 1) {
		pthread_mutex_lock(&tick_mutex);
			while (job_status[index] == 1) {
				pthread_cond_wait(&tick, &tick_mutex);
			}
			job_status[index] = 0;
		pthread_mutex_unlock(&tick_mutex);

		int start_execution_time = now();
		// wait to access job queue
		pthread_mutex_lock(&queue_mutex);
			// generate a random between 0 and 1
			float random = (rand() + 1.0) / (RAND_MAX+2.0);
			if (random > mu) {
				total_waiting_time = total_waiting_time + (now()-queue[begin].created_at);
				begin = begin + 1;
				total_execution_time = total_execution_time + (now()-start_execution_time);
				job_status[index] = 1;
				pthread_cond_signal(&all_done);
			}
		pthread_mutex_unlock(&queue_mutex);
	}

	pthread_exit(NULL);
}

void* clock_fn() {
	// initial tick to get out of deadlock
	pthread_cond_broadcast(&tick);

	int i; // tick 1000 times
	for( i = 0; i < ticks-1; i = i + 1 ) {
		clock_time = clock_time + 1;
		queue_length = queue_length + (end - begin + 1);
		pthread_mutex_lock(&all_done_mutex);
			// only tick if all threads are done with their jobs from the previous tick
			while (check_threads_done() == 0) {
				pthread_cond_wait(&all_done, &all_done_mutex);
			}

			// set all job status to 0
			reset_all();

			// tick
			pthread_cond_broadcast(&tick);			
		pthread_mutex_unlock(&all_done_mutex);
	}

	// the ticking officially ends
	program_done = 1;
	pthread_exit(NULL);
}

int main(int argc, char **argv) {
	if (argc != 9) {
		fprintf(stderr,"Error: Expected four arguments, got %d. Going to use default values.\n", argc);
      	lambda = 0.005, mu = 0.01;
		server_count = 2, client_count = 2;
	} else {
		int i =0;
		for(i=1;i<argc;i++) {
			if (strcmp("--servers", argv[i]) == 0) {
				server_count = atoi(argv[i + 1]);
			}
			if (strcmp("--clients", argv[i]) == 0) {
				client_count = atoi(argv[i + 1]);
			}
			if (strcmp("--lambda", argv[i]) == 0) {
				lambda = atof(argv[i + 1]);
			}
			if (strcmp("--mu", argv[i]) == 0) {
				mu = atof(argv[i + 1]);
			}
		} 
	}

	// instatiate variable values
	begin = 0, end = 0, length = 0, program_done = 0;
	total_waiting_time=0, total_jobs_ever=0, 
	total_execution_time=0, queue_length=0, clock_time=0;
	ticks = 1000, total_interarrival = 0, previous_arrival_time = 0;

	// allocate space for arrays
	servers = (pthread_t*)malloc(server_count*sizeof(pthread_t));
	clients = (pthread_t*)malloc(client_count*sizeof(pthread_t));
	job_status = (int*)malloc((client_count+server_count)*sizeof(int));
	capacity = ticks*client_count;
	queue = malloc(capacity*sizeof(job));

	// instantiate mutexes and condition variables
	pthread_mutex_init(&queue_mutex, NULL);
	pthread_mutex_init(&all_done_mutex, NULL);
	pthread_mutex_init(&tick_mutex, NULL);

	pthread_cond_init(&all_done, NULL);
	pthread_cond_init(&tick, NULL);

	int i = 0;
	reset_all();  // reset all clients and server to origin state


	// create clients
	for(i = 0; i < client_count; i = i + 1) {
		int err = pthread_create(&(clients[i]), NULL, client, (void *)(i));
		if (err != 0) {
	        printf("Could not create thread. Exit.\n  ");
	        exit(1);
	    }
	}

	// create servers
	for( i = 0; i < server_count; i = i + 1 ) {
		int err = pthread_create(&(servers[i]), NULL, &server, (void *)(i + client_count));
		if (err != 0) {
	        printf("Could not create thread. Exit.\n  ");
	        exit(1);
	    }
	}

	// create clock thread
	int err = pthread_create(&clock_thread, NULL, clock_fn, NULL);
	if (err != 0) {
        printf("Could not create thread. Exit.\n  ");
        exit(1);
    }

	// join all threads
	pthread_join(clock_thread, NULL);

	for(i = 0; i < client_count; i = i + 1) {
		pthread_join(clients[i], NULL);
	}

	for(i = 0; i < server_count; i = i + 1) {
		pthread_join(servers[i], NULL);
	}

	// destroy all mutexes and condition variables
	pthread_mutex_destroy(&queue_mutex);
	pthread_mutex_destroy(&all_done_mutex);
	pthread_mutex_destroy(&tick_mutex);

	pthread_cond_destroy(&all_done);
	pthread_cond_destroy(&tick);

    // calculate job statistics
	float avg_waiting_time = total_waiting_time / total_jobs_ever;
	float avg_exe_time = total_execution_time / total_jobs_ever;
	float avg_interarrival = total_interarrival / total_jobs_ever;
	float avg_turnaround_time = (total_waiting_time + total_execution_time) / total_jobs_ever;
	float avg_queue_length = queue_length / ticks;

	printf("Average waiting time (AWT)  %f. \n  ", avg_waiting_time);
	printf("Average execution time (AXT)  %f. \n  ", avg_exe_time);
	printf("Average turnaround time (ATA)  %f. \n  ", avg_turnaround_time);
	printf("Average queue length (AQL)  %f. \n  ", avg_queue_length);
	printf("Average interarrivaltime (AIA)  %f.\n", avg_interarrival);

	return 0;
}
