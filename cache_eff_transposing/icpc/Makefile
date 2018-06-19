
all:
	icpc generate_perm_vector.cpp -o generate_perm_vector -lm -O3

	icpc simple_transposing.cpp -o simple_transposing -lm -fopenmp -O3

	icpc  cache_eff_transposing.cpp -o cache_eff_transposing -fopenmp -O3
clean:
	rm -f *~ *.o
