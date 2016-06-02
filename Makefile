# -ffast-math - агрессивные оптимизации для арифметических вычислений
# -flto - межпоточные оптимизации
# -march=native - генерация кода под конкретный процессор с учетом архитектурных особенностей
# -mfpmath=sse - включает использование XMM регистров в вещественной арифметике (вместо вещественного стека в x87 режиме)
all:
	g++ -fopenmp -g landau-wang-omp-test.cpp -o landau-wang-omp -lboost_system -lboost_filesystem -mfpmath=sse -ffast-math -flto -march=native -xhost -O3
clean:
	rm landau-wang-omp
