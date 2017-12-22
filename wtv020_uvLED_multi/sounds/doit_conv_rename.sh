#for F in 0*.wav; do
#	sox -v 2.2 $F _$F
#done
for F in _*.wav; do
	new=`echo $F | sed 's/_//g'`
	cp $F $new
done
