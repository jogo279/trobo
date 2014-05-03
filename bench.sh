depth="7"
allflags="-v -d $depth"
seqminiflags="--minimax"
seqabflags="--ab"
parminiflags="-p --minimax"
parabflags="-p --ab"

exe="./MyTronBot"

echo "Maximum depth: $depth"
for i in maps/*
do
	if test -f "$i"
	then
		echo "$i at depth $depth"
		# echo "Parallel bot: "
		# cat "$i" | ./SeqTronBot -v

		echo "Sequential bot minimax: "
		out1=$($exe $allflags $seqminiflags < "$i" 2> errFile)
		cat errFile

		echo "Sequential bot with ab pruning: "
		out2=$($exe $allflags $seqabflags < "$i" 2> errFile)
		cat errFile

		echo "Parallel minimax bot: "
		out3=$($exe $allflags $parminiflags < "$i" 2> errFile)
		cat errFile

		echo "Parallel minimax bot: "
		out4=$($exe $allflags $seqminiflags < "$i" 2> errFile)
		cat errFile

		if [ "$out1" != "$out2" ]
		then
			echo "Outputs do not agree! Sequential bot: $out1 vs. Parallel bot: $out2"
			break
		fi
		if [ "$out1" != "$out3" ]
		then
			echo "Outputs do not agree! Sequential bot: $out1 vs. Parallel bot: $out3"
			break
		fi
		if [ "$out1" != "$out4" ]
		then
			echo "Outputs do not agree! Sequential bot: $out1 vs. Parallel bot: $out4"
			break
		fi
		echo "-----------------------------------"
	fi
done
rm -f errFile