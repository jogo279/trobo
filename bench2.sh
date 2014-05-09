seqminiflags="--minimax"
seqabflags="--ab"
parminiflags="-p --minimax"
parabflags="-p --ab"
parababortflags="-p --ab --abort"

exe="./MyTronBot"

echo "Maximum depth: $depth"
for depth in {5 6 7 8 9 10 11 12 13}
do
	echo "Depth $depth"
	for i in maps/*
	do
		if test -f "$i"
		then
			echo "Sequential bot minimax: "
			out1=$($exe "-v -d $depth" $seqminiflags < "$i" 2> errFile)
			cat errFile

			echo "Sequential ab bot: "
			out2=$($exe "-v -d $depth" $seqabflags < "$i" 2> errFile)
			cat errFile

			echo "Parallel minimax bot: "
			out3=$($exe "-v -d $depth" $parminiflags < "$i" 2> errFile)
			cat errFile

			echo "Parallel ab bot: "
			out4=$($exe "-v -d $depth" $parabflags < "$i" 2> errFile)
			cat errFile

			echo "Parallel ab abort bot: "
			out5=$($exe "-v -d $depth" $parababortflags < "$i" 2> errFile)
			cat errFile

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
			if [ "$out2" != "$out4" ]
			then
				echo "Outputs do not agree! Sequential ab bot: $out2 vs. Parallel ab bot: $out4"
				break
			fi
			if [ "$out2" != "$out5" ]
			then
				echo "Outputs do not agree! Sequential ab bot: $out2 vs. Parallel abort ab bot: $out5"
				break
			fi
			echo "-----------------------------------"
		fi
	done
done
rm -f errFile