all:
	g++ -fopenmp -g landau-wang-omp-test.cpp -o landau-wang-omp -lboost_system -lboost_filesystem -mfpmath=sse -ffast-math -flto -march=native -xhost -O3
clean:
	rm landau-wang-omp
