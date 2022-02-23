build:
	@gcc -o tema1_par tema1_par.c genetic_algorithm_par.c -lpthread -lm -Wall -Werror

run-t1: build
	time -p ./tema1_par ./inputs/in1 10 1
	time -p ./tema1_par ./inputs/in1 10 2
	time -p ./tema1_par ./inputs/in1 10 4  

run-t2: build
	time -p ./tema1_par ./inputs/in2 10 1
	time -p ./tema1_par ./inputs/in2 10 2
	time -p ./tema1_par ./inputs/in2 10 4

run-tests: run-t1 run-t2

clean:
	@rm -rf tema1_par
