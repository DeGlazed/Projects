#!/bin/bash

folder="sol"
tout=5
topo_tests=0
comp_tests=0
invalid_comm=0

# se ruleaza un test (parametru: comanda)
function run_test {
	timeout $tout $1 &> out.txt
	ret=$?

	if [ $ret == 124 ]
	then
		echo "W: Programul a durat prea mult"
	elif [ $ret != 0 ]
	then
		echo "W: Rularea nu s-a putut executa cu succes"
		cat out.txt
	fi
}

# se verifica topologia (parametru: numarul de procese)
function check_topology {
	local topology=`cat topology.txt`
	local procs=$(($1-1))
	local ok=0

	for i in `seq 0 $procs`
	do
		cat out.txt | grep -q -i "$i -> $topology"
		ok=$((ok+$?))
	done

	if [ $ok == 0 ]
	then
		topo_tests=$((topo_tests+10))
	else
		echo "E: Topologia nu este afisata corect de toate procesele"
	fi
}

# se verifica daca rezultatul calculelor este corect
function check_computation {
	local output=`cat output.txt`
	local ok=0

	cat out.txt | grep -q -i "$output"
	
	if [ $? == 0 ]
	then
		comp_tests=$((comp_tests+10))
	else
		echo "E: Rezultatul final nu este corect"
		echo "Se astepta:"
		echo $output
		echo "S-a gasit:"
		cat out.txt | grep "Rezultat"
	fi
}


cd $folder
rm -rf tema3

# se compileaza
make clean
make build

if [ ! -f tema3 ]
then
    echo "E: Nu s-a putut compila tema"
    show_score
    exit
fi

# se ruleaza 4 teste
for i in `seq 1 4`
do
	echo ""
	echo "Se ruleaza testul $i..."

	cp ../tests/test$i/*.txt .

	procs=`sed '1q;d' inputs.txt`
	N=`sed '2q;d' inputs.txt`

	run_test "mpirun -np $procs --oversubscribe ./tema3 $N 0"

	# verificare topologie
	check_topology $procs

	# verificare rezultat calcul
	check_computation

	rm -rf *.txt
done