#!/bin/bash

tout=3
folder="skel"


# se ruleaza un test (parametru: comanda)
function run_test {
	time $1
}

# se compara doua fisiere (parametri: fisier1 fisier2)
function compare_outputs {
	diff -wq $1 $2 &> /dev/null
	if [ $? == 0 ]
	then
		echo "Nu s-au gasit diferente intre output si file verificare"
	else
		echo "W: Exista diferente intre fisierele $1 si $2"
		echo "Se astepta:"
		cat $1
		echo "S-a gasit:"
		cat $2
	fi
}

function cleanup {
	rm -rf *.txt
	rm -rf *.class
	rm -rf tests
	cd ..
}

cp -R tests $folder/

cd $folder
rm -rf *.class

javac *.java

if [ ! -f Tema2.class ]
then
    echo "E: Nu s-a putut compila"
    cleanup
    exit
fi

rm -rf *.txt

# se ruleaza cele 5 teste cu numar diferiti de workeri
for test in 0 1 2 3 4
do
	for workers in 1 2 3 4
	do
		echo "Se ruleaza testul $test cu $workers worker(i)"
		run_test "java Tema2 $workers tests/in/test${test}.txt test${test}_out.txt"
		compare_outputs tests/out/test${test}_out.txt test${test}_out.txt
		rm -rf test${test}_out.txt
	done
done

cleanup

