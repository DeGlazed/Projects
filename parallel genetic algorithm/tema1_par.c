#include <stdlib.h>
#include "genetic_algorithm_par.h"
#include <stdio.h>
#include <pthread.h>

int main(int argc, char *argv[]) {
	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;

	// number of objects
	int object_count = 0;

	// maximum weight that can be carried in the sack
	int sack_capacity = 0;

	// number of generations
	int generations_count = 0;

	// number of threads
	int P = 0;
	int r;

	if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, &P, argc, argv)) {
		return 0;
	}

	pthread_t threads[P];
	void *status;
	pthread_barrier_t bar;
	pthread_barrier_init(&bar, NULL, P); 
	pthread_mutex_t mutex; 
	pthread_mutex_init(&mutex, NULL);

	//comute initial generation
	individual *current_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *next_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *tmp = NULL;

	for (int i = 0; i < object_count; ++i) {
		current_generation[i].fitness = 0;
		current_generation[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		current_generation[i].chromosomes[i] = 1;
		current_generation[i].index = i;
		current_generation[i].chromosome_length = object_count;
		current_generation[i].ones = 1;

		next_generation[i].fitness = 0;
		next_generation[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		next_generation[i].index = i;
		next_generation[i].chromosome_length = object_count;
	}

	//compute args
	data_struct func_args[P];
	for(int i = 0; i < P; i++){
		func_args[i].thread_ID = i;
		func_args[i].generations_count = generations_count;
		func_args[i].object_count = object_count;
		func_args[i].objects = objects;
	    func_args[i].sack_capacity = sack_capacity;
		func_args[i].mutex = &mutex;
		func_args[i].bar = &bar;
		func_args[i].current_generation = current_generation;
		func_args[i].next_generation = next_generation;
		func_args[i].tmp = tmp;
		func_args[i].thread_count = P;
	}


	//open threads
	for (int i = 0; i < P; i++) {
		r = pthread_create(&threads[i], NULL, run_genetic_algorithm_thread, &func_args[i]);

		if (r) {
			printf("Eroare la crearea thread-ului %d\n", i);
			exit(-1);
		}
	}

	//join threads
	for (int i = 0; i < P; i++) {
		r = pthread_join(threads[i], &status);

		if (r) {
			printf("Eroare la asteptarea thread-ului %d\n", i);
			exit(-1);
		}
	}

	pthread_barrier_destroy(&bar);
	pthread_mutex_destroy(&mutex);
	free(objects);

	return 0;
}
