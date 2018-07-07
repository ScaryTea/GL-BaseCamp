#!/bin/bash
res_path="./lee_result_O"
exe_path="./leeO"
NUM=1
ind=(0 1 2 3 s)
i=0

while [ $i -lt ${#ind[*]} ]
do
	echo "i = "$i
	while [ $NUM -lt 2000 ]
	do
		$exe_path${ind[i]} $NUM $NUM >> $res_path${ind[i]}
		echo "current argument is: "$NUM
		let NUM=NUM+40
	done
	let i=i+1
	NUM=1
done
