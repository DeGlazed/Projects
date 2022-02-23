#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "genetic_algorithm_par.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int *P, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 4) {
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count no_threads\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int) strtol(argv[2], NULL, 10);
	*P = (int) strtol(argv[3], NULL, 10);
	
	if (*generations_count == 0) {
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i) {
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function_in_range(const sack_object *objects, individual *generation, int start, int end, int sack_capacity)
{
	int weight;
	int profit;
	int ones;

	for (int i = start; i < end; ++i) {
		weight = 0;
		profit = 0;
		ones = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			if (generation[i].chromosomes[j]) {
				weight += objects[j].weight;
				profit += objects[j].profit;
				ones ++;
			}
		}
		generation[i].ones = ones;
		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

int cmpfunc(const void *a, const void *b)
{
	//int i;
	individual *first = (individual *) a;
	individual *second = (individual *) b;

	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0) {

		int first_count = first->ones, second_count = second->ones;

		// for (i = 0; i < first->chromosome_length && i < second->chromosome_length; ++i) {
		// 	first_count += first->chromosomes[i];
		// 	second_count += second->chromosomes[i];
		// }

		res = first_count - second_count; // increasing by number of objects in the sack
		if (res == 0) {
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) {
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} else {
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step) {
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(individual *from, individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation_in_range(individual *generation, int start, int end)
{
	int i;

	for (i = start; i < end; ++i) {
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}

void merge(individual *source, int start, int mid, int end, individual *destination) {
	int iA = start;
	int iB = mid;
	int i;

	for (i = start; i < end; i++) {
		if (end == iB || (iA < mid && (source + iA)->fitness >= (source + iB)->fitness)) {
			copy_individual(source + iA, destination + i);
			iA++;
		} else {
			copy_individual(source + iB, destination + i);
			iB++;
		}
	}
}

void *run_genetic_algorithm_thread(void *data)
{
	data_struct arg = *(data_struct *)data;
	int start_idx = arg.thread_ID * (double)arg.object_count / arg.thread_count;
	int end_idx = MIN((arg.thread_ID + 1) * (double)arg.object_count / arg.thread_count , arg.object_count);

	// iterate for each generation
	for (int k = 0; k < arg.generations_count; ++k) {

		pthread_barrier_wait(arg.bar);

		int count = 0;
		int cursor = 0;

		//-------------------FITNESS
		// compute fitness and sort by it
		int start = arg.thread_ID * (double)arg.object_count / arg.thread_count;
		int end = MIN((arg.thread_ID + 1) * (double)arg.object_count / arg.thread_count , arg.object_count);
		compute_fitness_function_in_range(arg.objects, arg.current_generation, start, end, arg.sack_capacity);
		pthread_barrier_wait(arg.bar);
		//--------------------------

		//----------------------SORT
		if(arg.thread_ID == 0)
			qsort(arg.current_generation, arg.object_count, sizeof(individual), cmpfunc);
		pthread_barrier_wait(arg.bar);
		//--------------------------

		//------------------KEEP
		// keep first 30% children (elite children selection)
		count = arg.object_count * 3 / 10;
		start = arg.thread_ID * (double)count / arg.thread_count;
		end = MIN((arg.thread_ID + 1) * (double)count / arg.thread_count , count);
		for (int i = start; i < end; ++i) {
			copy_individual(arg.current_generation + i, arg.next_generation + i);
		}
		cursor = count;
		//----------------------

		//---------------MUATE 1
		// mutate first 20% children with the first version of bit string mutation
		count = arg.object_count * 2 / 10;
		start = arg.thread_ID * (double)count / arg.thread_count;
		end = MIN((arg.thread_ID + 1) * (double)count / arg.thread_count , count);
		for (int i = start; i < end; ++i) {
	 		copy_individual(arg.current_generation + i, arg.next_generation + cursor + i);
	 		mutate_bit_string_1(arg.next_generation + cursor + i, k);
	 	}
	 	cursor += count;
		//----------------------

		//--------------MUTATE 2
	 	// mutate next 20% children with the second version of bit string mutation
	 	count = arg.object_count * 2 / 10;
		start = arg.thread_ID * (double)count / arg.thread_count;
		end = MIN((arg.thread_ID + 1) * (double)count / arg.thread_count , count);
	 	for (int i = start; i < end; ++i) {
	 		copy_individual(arg.current_generation + i + count, arg.next_generation + cursor + i);
	 		mutate_bit_string_2(arg.next_generation + cursor + i, k);
	 	}
	 	cursor += count;
		//----------------------

		//-------------CROSSOVER
		// crossover first 30% parents with one-point crossover
		// (if there is an odd number of parents, the last one is kept as such)
	 	count = arg.object_count * 3 / 10;
		if (count % 2 == 1) {
			if(arg.thread_ID == 0)
	 			copy_individual(arg.current_generation + arg.object_count - 1, arg.next_generation + cursor + count - 1);
	 		count--;
	 	}
		pthread_barrier_wait(arg.bar);

		start = arg.thread_ID * (double)count / arg.thread_count;
		end = MIN((arg.thread_ID + 1) * (double)count / arg.thread_count , count);

		if(start%2 == 1) {
			start += 1;
		}

	 	for (int i = start; i < end; i += 2) {
	 		crossover(arg.current_generation + i, arg.next_generation + cursor + i, k);
	 	}
		pthread_barrier_wait(arg.bar);
		//----------------------

		//-------------SWITCH GEN
		if(arg.thread_ID == 0){
			arg.tmp = arg.current_generation;
			arg.current_generation = arg.next_generation;
			arg.next_generation = arg.tmp;
		}
		pthread_barrier_wait(arg.bar);
		//-------------------------

		//------------UPDATE GEN INDEX
		start = arg.thread_ID * (double)arg.object_count / arg.thread_count;
		end = MIN((arg.thread_ID + 1) * (double)arg.object_count / arg.thread_count , arg.object_count);
	 	for (int i = start; i < end; ++i) {
	 		arg.current_generation[i].index = i;
	 	}
		pthread_barrier_wait(arg.bar);

	 	if (k % 5 == 0 && arg.thread_ID == 0) {
	 		print_best_fitness(arg.current_generation);
	 	}
	}

	compute_fitness_function_in_range(arg.objects, arg.current_generation, start_idx, end_idx, arg.sack_capacity);
	if(arg.thread_ID == 0)
			qsort(arg.current_generation, arg.object_count, sizeof(individual), cmpfunc);
	pthread_barrier_wait(arg.bar);

	if(arg.thread_ID == 0) {
		print_best_fitness(arg.current_generation);
	}

	free_generation_in_range(arg.current_generation, start_idx, end_idx);
	free_generation_in_range(arg.next_generation, start_idx, end_idx);
	pthread_exit(NULL);
}