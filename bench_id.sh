allflags="-v"
seqminiflags="--minimax"
seqabflags="--ab"
parminiflags="-p --minimax"
parabflags="-p --ab"
parababortflags="-p --ab --abort"
hybridflags="-p --hybrid"

exe="./MyTronBot"

echo "Maximum depth: $depth"
for i in maps/*
do
	if test -f "$i"
	then
		echo "$i"
		# echo "Parallel bot: "
		# cat "$i" | ./SeqTronBot -v

		# echo "Sequential bot minimax: "
		# out1=$($exe $allflags $seqminiflags < "$i" 2> errFile)
		# cat errFile

		# echo "Parallel minimax bot: "
		# out3=$($exe $allflags $parminiflags < "$i" 2> errFile)
		# cat errFile

		echo "Sequential ab bot: "
		out2=$($exe $allflags $seqabflags < "$i" 2> errFile)
		cat errFile

		echo "Parallel ab bot: "
		out4=$($exe $allflags $parabflags < "$i" 2> errFile)
		cat errFile

		echo "Parallel ab abort bot: "
		out5=$($exe $allflags $parababortflags < "$i" 2> errFile)
		cat errFile
		
		# echo "Hybrid bot: "
		# out6=$($exe $allflags $hybridflags < "$i" 2> errFile)
		# cat errFile

		# if [ "$out1" != "$out2" ]
		# then
		# 	echo "Outputs do not agree! Sequential minimax bot: $out1 vs. sequential ab bot: $out2"
		# 	break
		# fi
		# if [ "$out1" != "$out3" ]
		# then
		# 	echo "Outputs do not agree! Sequential minimax bot: $out1 vs. Parallel minimax bot: $out3"
		# 	break
		# fi
		# if [ "$out1" != "$out4" ]
		# then
		# 	echo "Outputs do not agree! Sequential minimax bot: $out1 vs. Parallel ab bot: $out4"
		# 	break
		# fi
		# if [ "$out2" != "$out4" ]
		# then
		# 	echo "Outputs do not agree! Sequential ab bot: $out2 vs. Parallel ab bot: $out4"
		# 	break
		# fi
		# if [ "$out2" != "$out5" ]
		# then
		# 	echo "Outputs do not agree! Sequential ab bot: $out2 vs. Parallel abort ab bot: $out4"
		# 	break
		# fi
		echo "-----------------------------------"
	fi
done
rm -f errFile