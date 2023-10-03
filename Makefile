
build:
	mpicxx -fopenmp -c  main.c -o main.o
	mpicxx -fopenmp -c cFunctions.c -o cFunctions.o
	nvcc -gencode arch=compute_61,code=sm_61 -c cudaFunctions.cu -o cudaFunctions.o
	#linking:
	mpicxx  -fopenmp -o mpiCudaOpenMP  main.o cFunctions.o cudaFunctions.o  -L/usr/local/cuda/lib -L/usr/local/cuda/lib64 -lcudart
	
clean:
	rm -f *.o ./mpiCudaOpenMP
	rm -f *.btr
	rm -f MP

run:
	mpiexec -n 3 ./mpiCudaOpenMP <data.txt 


normal:
	mpicxx -fopenmp -c   cNormal_main.c -o main.o
	mpicxx -fopenmp -c cFunctions.c -o cFunctions.o
	mpicxx  -fopenmp -o MP  main.o cFunctions.o

normal_run:
	mpiexec -n 1 ./MP matrix.txt <data.txt >result.txt