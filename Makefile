all:
	g++ -fopenmp -g landau-wang-omp-test.cpp -o landau-wang-omp -lboost_system -lboost_filesystem -xhost -O1
clean:
	rm landau-wang-omp
