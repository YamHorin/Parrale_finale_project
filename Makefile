
build:
	mpicxx -fopenmp -c main.c -o main.o
	mpicxx -fopenmp -c cFunctions.c -o cFunctions.o
	nvcc -gencode arch=compute_61,code=sm_61 -c cudaFunctions.cu -o cudaFunctions.o
	#linking:
	mpicxx  -fopenmp -o mpiCudaOpenMP  main.o cFunctions.o cudaFunctions.o  -L/usr/local/cuda/lib -L/usr/local/cuda/lib64 -lcudart
	
clean:
	rm -f *.o ./mpiCudaOpenMP
	rm -f *.btr
	rm -f MP
	rm -f result_parallel.txt
	rm -f result_seq.txt

run:
	mpiexec -n 5 ./mpiCudaOpenMP grade_table.txt <data2.txt > result_parallel.txt


normal:
	mpicxx -fopenmp -c   cNormal_main.c -o main.o
	mpicxx -fopenmp -c cFunctions.c -o cFunctions.o
	mpicxx  -fopenmp -o MP  main.o cFunctions.o

normal_run:
	mpiexec -n 1 ./MP grade_table.txt <data2.txt >result_seq.txt