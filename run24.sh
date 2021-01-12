for i in {1..23}
do
	./prover $i 12345 &
	sleep 0.05
done

time ./prover 24 12345 &
