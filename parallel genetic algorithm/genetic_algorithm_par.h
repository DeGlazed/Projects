#ifndef GENETIC_ALGORITHM_PAR_H
#define GENETIC_ALGORITHM_PAR_H

#include "sack_object.h"
#include "individual.h"
#include <pthread.h>

typedef struct data_struct
{
    int thread_ID;
    const sack_object *objects;
    int object_count;
    int thread_count;
    int generations_count;
    int sack_capacity;
    pthread_mutex_t *mutex;
    pthread_barrier_t *bar;
    individual *current_generation;
	individual *next_generation;
	individual *tmp;

} data_struct;

// reads input from a given file
int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int *P, int argc, char *argv[]);

// displays all the objects that can be placed in the sack
void print_objects(const sack_object *objects, int object_count);

// displays all or a part of the individuals in a generation
void print_generation(const individual *generation, int limit);

// displays the individual with the best fitness in a generation
void print_best_fitness(const individual *generation);

// compares two individuals by fitness and then number of objects in the sack (to be used with qsort)
int cmpfunc(const void *a, const void *b);

// performs a variant of bit string mutation
void mutate_bit_string_1(const individual *ind, int generation_index);

// performs a different variant of bit string mutation
void mutate_bit_string_2(const individual *ind, int generation_index);

// performs one-point crossover
void crossover(individual *parent1, individual *child1, int generation_index);

// copies one individual
void copy_individual(individual *from, individual *to);

//----------------------------------paralel--------------------------
void *run_genetic_algorithm_thread(void *data);
void compute_fitness_function_in_range(const sack_object *objects, individual *generation, int start, int end, int sack_capacity);
void free_generation_in_range(individual *generation, int start, int end);
void merge(individual *source, int start, int mid, int end, individual *destination);

#endif