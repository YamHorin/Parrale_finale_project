#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <cuda_runtime.h> // Include CUDA runtime header

#define MATRIX_SIZE 26
#define BLOCK_DIM 1024 // number of threads in a block

__device__ int matrix_caculate[MATRIX_SIZE * MATRIX_SIZE];
__device__ char first_str[BLOCK_DIM];
__device__ int length_first_str;


__device__ int getScoreFromMatrix(char a, char b) {
    int x = a - 'A';
    int y = b - 'A';
    return matrix_caculate[x * MATRIX_SIZE + y]; // Assuming matrix_caculate is a 1D array representation of a 2D matrix.
}


__device__ void scan_plus(int *array, int size)
{
   for (unsigned int stride=1; stride <= size/2; stride *= 2) {
        int v;
        if (threadIdx.x >= stride) {
            v = array[threadIdx.x - stride];
        }
        __syncthreads(); /* wait until all threads finish reading 
		                    an element */

        if (threadIdx.x >= stride)
            array[threadIdx.x] += v;

        __syncthreads(); /* wait until all threads finish updating an
		                    element */
     }
     
} // scan_plus




__global__ void caculateWithMatrix(const char  *s1, int n1, const char *s2, int n2,  int *result)
{
     __shared__ int r;
     int value;
     int tid = threadIdx.x;
    if(tid == 0) r = 0;
    __syncthreads();
    if (tid < n1 && tid <n2)
    {
        value = (getScoreFromMatrix (s1[tid] ,s2[tid])); 
        atomicAdd(&r , value);

    }
        

     //scan_plus(flags, BLOCK_DIM);
     //if (tid  == BLOCK_DIM-1; 
     __syncthreads();
     if (tid == 0)
        *result = r;

}
__global__ void caculate(const char  *s1, int n1, const char *s2, int n2,  int *result)
{
    // __shared__ int flags[BLOCK_DIM];
     __shared__ int r;
     int value;
     int tid = threadIdx.x;
    if(tid == 0) r = 0;
    __syncthreads();
    if (tid < n1 && tid <n2)
    {
        value = (s1[tid] == s2[tid]); 
        atomicAdd(&r , value);

    }
        

     //scan_plus(flags, BLOCK_DIM);
     //if (tid  == BLOCK_DIM-1; 
     __syncthreads();
     if (tid == 0)
        *result = r;

}

// returns 0 if successful, otherwise returns 1
// int computeOnGPU(const char *s2) {
//     char *dev_s2;
//     int *dev_result;
//      // null byte at the end is also counted
//     int n2 = strlen(s2)+1;
//     // allocate the memory on the GPU
//     cudaError_t err2 = cudaMalloc((void**)&dev_s2, n2);
//     if (err2 != cudaSuccess)
//     {
//         fprintf(stderr, "CUDA  2 error\n");
//         exit(1);
//     }
//     cudaError_t err3 =  cudaMalloc((void**)&dev_result, sizeof(int));
//     if (err3 != cudaSuccess)
//     {
//         fprintf(stderr, "CUDA 3 error\n");
//         exit(1);
//     }
//     cudaMemcpy(dev_s2, s2, n2, cudaMemcpyHostToDevice);
//     int threadsPerBlock = BLOCK_DIM;
//     int numOfBlocks = 1;
//     //if strlen <1024
//     caculate<<<numOfBlocks, threadsPerBlock>>>(first_str,lenght_first_str,dev_s2, n2, dev_result);
//     err1 = cudaGetLastError();
//     if (err1 != cudaSuccess)
//     {
//         fprintf(stderr , "kerner lanch error\n");
//         exit(1);
//     }
//     // copy the result back from the GPU to the CPU
//     int result;
//     err1 = cudaMemcpy(&result, dev_result, sizeof(int), cudaMemcpyDeviceToHost);		
//     if (err1 != cudaSuccess)
//     {
//         fprintf(stderr , "kerner lanch error");
//         exit(1);
//     }
//     // free memory on the GPU side
//     cudaFree(dev_s1);
//     cudaFree(dev_s2);
//     cudaFree(dev_result);
//     return result;
    
// }
// int computeOnGPUWithMatrix(const char  *s1, const char *s2 ,const int matrix[MATRIX_SIZE][MATRIX_SIZE])
// {
//     cudaMemcpyToSymbol(matrix_caculate, matrix, MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
//     char *dev_s1, *dev_s2;
//     int *dev_result;
//     int n1 = strlen(s1)+1; // null byte at the end is also counted
//     int n2 = strlen(s2)+1;

//     // allocate the memory on the GPU
//     cudaError_t err1 = cudaMalloc((void**)&dev_s1, n1);
//     cudaError_t err2 = cudaMalloc((void**)&dev_s2, n2);
//     cudaError_t err3 =  cudaMalloc((void**)&dev_result, sizeof(int));
//     if(err1 != cudaSuccess || err2 != cudaSuccess || err3 != cudaSuccess) {
//         fprintf(stderr, "CUDA error\n");
//         exit(1);
//     }
//     cudaMemcpy(dev_s1, s1, n1, cudaMemcpyHostToDevice);
//     cudaMemcpy(dev_s2, s2, n2, cudaMemcpyHostToDevice);
    
//     int threadsPerBlock = BLOCK_DIM;
//     int numOfBlocks = 1;
 
//     caculateWithMatrix<<<numOfBlocks, threadsPerBlock>>>(dev_s1, n1, dev_s2, n2, dev_result);
 
//     // copy the result back from the GPU to the CPU
//     int result=0;
//     cudaMemcpy(&result, dev_result, sizeof(int), cudaMemcpyDeviceToHost);		
	    
//     // free memory on the GPU side
//     cudaFree(dev_s1);
//     cudaFree(dev_s2);
//     cudaFree(dev_result);

//     return result;
// }
void getFirstStr(char *s1, int n1)
{
    cudaMemcpyToSymbol(first_str, s1, n1 * sizeof(char));
    cudaMemcpyToSymbol(length_first_str, &n1, sizeof(int));
}

__global__ void change_offset(char *str, int offset)
{
    int tid = threadIdx.x;
    if (tid < length_first_str)
    {
        str[tid] = first_str[tid + offset];
    }
    if (tid == length_first_str)
    {
        str[tid] = '\0';
    }
}

char *offsetFirstStr(int offset , int lenght)
{
    char *result, *returnStr;
    cudaMalloc((void **)&result, lenght);
    int threadsPerBlock = BLOCK_DIM;
    int numOfBlocks = 1;
    change_offset<<<numOfBlocks, threadsPerBlock>>>(result, offset);
    returnStr = (char *)malloc(lenght * sizeof(char));
    cudaMemcpy(returnStr, result, lenght * sizeof(char), cudaMemcpyDeviceToHost);
    cudaFree(result);
    return returnStr;
}

