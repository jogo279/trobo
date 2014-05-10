depth="10"
allflags="-v -d $depth"
seqminiflags="--minimax"
seqabflags="--ab"
parminiflags="-p --minimax"
parabflags="-p --ab"
parababortflags="-p --ab --abort"
hybridflags="-p --hybrid"

exe="./MyTronBot"

tot1="0"
tot2="0"
tot3="0"
tot4="0"
tot5="0"
tot6="0" 

numfiles=$(ls maps | wc -l)
echo "$numfiles"
for i in maps/*
do
	if test -f "$i"
	then
		echo "$i at depth $depth"
		# echo "Parallel bot: "
		# cat "$i" | ./SeqTronBot -v

		# echo "Sequential bot minimax: "
		# out1=$($exe $allflags $seqminiflags < "$i" 2> errFile)
		# cat errFile
		# tmp1=$(cat errFile)
		# tot1=$(perl -e "print $tot1+$tmp1")

		echo "Sequential ab bot: "
		out2=$($exe $allflags $seqabflags < "$i" 2> errFile)
		cat errFile
		tmp2=$(cat errFile)
		tot2=$(perl -e "print $tot2+$tmp2")

		# echo "Parallel minimax bot: "
		# out3=$($exe $allflags $parminiflags < "$i" 2> errFile)
		# cat errFile
		# tmp3=$(cat errFile)
		# tot3=$(perl -e "print $tot3+$tmp3")

		echo "Parallel ab bot: "
		out4=$($exe $allflags $parabflags < "$i" 2> errFile)
		cat errFile
		tmp4=$(cat errFile)
		spd4=$(perl -e "print ($tmp2/$tmp4)")
		echo "$spd4"
		tot4=$(perl -e "print $tot4+$spd4")

		echo "Parallel ab abort bot: "
		out5=$($exe $allflags $parababortflags < "$i" 2> errFile)
		cat errFile
		tmp5=$(cat errFile)
		spd5=$(perl -e "print ($tmp2/$tmp5)")
		echo "$spd5"
		tot5=$(perl -e "print $tot5+$spd5")

		echo "Hybrid bot: "
		out6=$($exe $allflags $hybridflags < "$i" 2> errFile)
		cat errFile
		echo "$out6"
		tmp6=$(cat errFile)
		spd6=$(perl -e "print ($tmp2/$tmp6)")
		echo "$spd6"
		tot6=$(perl -e "print $tot6+$spd6")


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
rm -f errFile

echo "Sequential minimax total: $tot1"
echo "Sequential ab total: $tot2"
echo "Parallel minimax total: $tot3"
echo "Parallel ab total: $tot4"
echo "Parallel abort ab total: $tot5"
echo "Hybrid total: $tot6"