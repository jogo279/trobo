depth="7"
flags="-v --ab -d $depth"
exe="./MyTronBot"

echo "Maximum depth: $depth"
for i in maps/*
do
	if test -f "$i"
	then
		echo "$i at depth $depth"
		# echo "Parallel bot: "
		# cat "$i" | ./SeqTronBot -v
		echo "Sequential bot with ab pruning: "
		out1=$($exe $flags < "$i" 2> errFile)
		cat errFile

		echo "Parallel bot: "
		out2=$($exe $flags -p < "$i" 2> errFile)
		cat errFile

		if [ "$out1" != "$out2" ]
		then
			echo "Outputs do not agree! Sequential bot: $out1 vs. Parallel bot: $out2"
			break
		fi
		echo "-----------------------------------"
	fi
done
rm -f errFile