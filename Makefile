# -ffast-math - агрессивные оптимизации для арифметических вычислений
# -flto - межпоточные оптимизации
# -march=native - генерация кода под конкретный процессор с учетом архитектурных особенностей
# -mfpmath=sse - включает использование XMM регистров в вещественной арифметике (вместо вещественного стека в x87 режиме) (Имеет смысл только в 32х битном режиме! В 64 смысла не имеет, т.к. используется SSE2)
# -funroll-loop - включает развертывание циклов 

all:
	g++ -fopenmp main.cpp landau-wang-omp-3d.cpp landau-wang-omp-2d.cpp make-scripts-to-plot.cpp mersenne.cpp -o landau-wang-omp -lboost_system -lboost_filesystem -ffast-math -flto -march=native -msse4.2 -xhost -O3
#	g++ -fopenmp main.cpp landau-wang-omp-3d.cpp landau-wang-omp-2d.cpp make-scripts-to-plot.cpp mersenne.cpp -o landau-wang-omp -lboost_system -lboost_filesystem -march=native -msse4.2 -xhost -O3
	#g++ -fopenmp -g landau-wang-omp-3d.cpp -o landau-wang-omp-3d -lboost_system -lboost_filesystem -ffast-math -flto -march=native -msse4.2 -xhost -O3
clean:
	rm landau-wang-omp
