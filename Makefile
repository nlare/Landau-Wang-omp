# -ffast-math - агрессивные оптимизации для арифметических вычислений
# -flto - межпоточные оптимизации
# -march=native - генерация кода под конкретный процессор с учетом архитектурных особенностей
# -mfpmath=sse - включает использование XMM регистров в вещественной арифметике (вместо вещественного стека в x87 режиме) (Имеет смысл только в 32х битном режиме! В 64 смысла не имеет, т.к. используется SSE2)
# -funroll-loop - включает развертывание циклов 

all:
	g++ -fopenmp -g landau-wang-omp-2d.cpp -o landau-wang-omp-2d -lboost_system -lboost_filesystem -ffast-math -flto -march=native -funroll-loops -xhost -O3
	g++ -fopenmp -g landau-wang-omp-3d.cpp -o landau-wang-omp-3d -lboost_system -lboost_filesystem -ffast-math -flto -march=native -funroll-loops -xhost -O3
2d:
	g++ -fopenmp -g landau-wang-omp-2d.cpp -o landau-wang-omp-2d -lboost_system -lboost_filesystem -ffast-math -flto -march=native -funroll-loops -xhost -O3
3d:
	g++ -fopenmp -g landau-wang-omp-3d.cpp -o landau-wang-omp-3d -lboost_system -lboost_filesystem -ffast-math -flto -march=native -funroll-loops -xhost -O3
clean:
	rm landau-wang-omp-3d landau-wang-omp-2d
