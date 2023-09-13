
build:
	mpicxx -fopenmp -c  -DDEBUG main.c -o main.o
	mpicxx -fopenmp -c cFunctions.c -o cFunctions.o
	nvcc -gencode arch=compute_61,code=sm_61 -c cudaFunctions.cu -o cudaFunctions.o
	#linking:
	mpicxx  -fopenmp -o mpiCudaOpenMP  main.o cFunctions.o cudaFunctions.o  -L/usr/local/cuda/lib -L/usr/local/cuda/lib64 -lcudart
	
clean:
	rm -f *.o ./mpiCudaOpenMP
	rm -f result.txt	
	rm -f *.btr

run:
	mpiexec -n 4 ./mpiCudaOpenMP <data.txt 




