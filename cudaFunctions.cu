#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <cuda_runtime.h> // Include CUDA runtime header

#define BLOCK_DIM 1024 // number of threads in a block
#define MAX_STRING_SIZE 3000
#define MATRIX_SIZE 26


__device__ char gpu_toupper(char c)
{
    if (c>='a' && c<='A')
        return c-('a'-'A');
    return c;
}
// __device__ int getScoreFromMatrix(char a, char b) {
//     int x = a - 'A';
//     int y = b - 'A';
//     return matrix_caculate[x * MATRIX_SIZE + y]; // Assuming matrix_caculate is a 1D array representation of a 2D matrix.
// }


// __device__ void scan_plus(int *array, int size)
// {
//    for (unsigned int stride=1; stride <= size/2; stride *= 2) {
//         int v;
//         if (threadIdx.x >= stride) {
//             v = array[threadIdx.x - stride];
//         }
//         __syncthreads(); /* wait until all threads finish reading 
// 		                    an element */

//         if (threadIdx.x >= stride)
//             array[threadIdx.x] += v;

//         __syncthreads(); /* wait until all threads finish updating an
// 		                    element */
//      }
     
// } // scan_plus




// __global__ void caculateWithMatrix(const char  *s1, const char *s2, int n2,  int *result , int off_set)
// {
//      __shared__ int r;
//      int value;
//      int tid = threadIdx.x;
//     if(tid == 0) r = 0;
//     __syncthreads();
//     if ((tid +off_set)< n2)
//     {
//         value = (getScoreFromMatrix (s1[tid +off_set] ,s2[tid])); 
//         atomicAdd(&r , value);

//     }
//      //scan_plus(flags, BLOCK_DIM);
//      //if (tid  == BLOCK_DIM-1; 
//      __syncthreads();
//      if (tid == 0)
//         *result = r;

// }
// __global__ void caculate(const char  *s1, const char *s2, int n2,  int *result , int off_set)
// {
//     // __shared__ int flags[BLOCK_DIM];
//      __shared__ int r;
//      int value;
//      int tid = threadIdx.x;
//     if(tid == 0) r = 0;
//     __syncthreads();
//     if ((tid +off_set) < n2)
//     {
//         value = (s1[tid +off_set] == s2[tid]); 
//         atomicAdd(&r , value);

//     }
        

//      //scan_plus(flags, BLOCK_DIM);
//      //if (tid  == BLOCK_DIM-1; 
//      __syncthreads();
//      if (tid == 0)
//         *result = r;

// }


// int computeOnGPU(const char *s2 , int off_set) {
//     char *dev_s2;
//     int *dev_result;
//      // null byte at the end is also counted
//     int n2 = strlen(s2);
//     // allocate the memory on the GPU
//     cudaError_t err2 = cudaMalloc((void**)&dev_s2, n2);
//     if (err2 != cudaSuccess)
//     {
//         fprintf(stderr, "CUDA  1 error\n");
//         exit(1);
//     }
//     cudaError_t err3 =  cudaMalloc((void**)&dev_result, sizeof(int));
//     if (err3 != cudaSuccess)
//     {
//         fprintf(stderr, "CUDA 2 error\n");
//         exit(1);
//     }
//     cudaMemcpy(dev_s2, s2, n2, cudaMemcpyHostToDevice);
//     int threadsPerBlock = BLOCK_DIM;
//     int numOfBlocks = 1;
//     //if strlen <1024
//     caculate<<<numOfBlocks, threadsPerBlock>>>(first_str,dev_s2, n2, dev_result ,off_set);
//     err2 = cudaGetLastError();
//     if (err2 != cudaSuccess)
//     {
//         fprintf(stderr , "kerner lanch error\n");
//         exit(1);
//     }
//     // copy the result back from the GPU to the CPU
//     int result;
//     err2 = cudaMemcpy(&result, dev_result, sizeof(int), cudaMemcpyDeviceToHost);		
//     if (err2 != cudaSuccess)
//     {
//         fprintf(stderr , "kerner lanch error");
//         exit(1);
//     }
//     // free memory on the GPU side
//     cudaFree(dev_s2);
//     cudaFree(dev_result);
//     return result;
    
// }
// int computeOnGPUWithMatrix( const char *s2 ,const int matrix[MATRIX_SIZE][MATRIX_SIZE] , int off_set)
// {
//     cudaMemcpyToSymbol(matrix_caculate, matrix, MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
//     char *dev_s2;
//     int *dev_result;
//     int n2 = strlen(s2);

//     // allocate the memory on the GPU
//     cudaError_t err2 = cudaMalloc((void**)&dev_s2, n2);
//     cudaError_t err3 =  cudaMalloc((void**)&dev_result, sizeof(int));
//     if(err2 != cudaSuccess || err3 != cudaSuccess) {
//         fprintf(stderr, "CUDA error\n");
//         exit(1);
//     }
//     cudaMemcpy(dev_s2, s2, n2, cudaMemcpyHostToDevice);
    
//     int threadsPerBlock = BLOCK_DIM;
//     int numOfBlocks = n2/BLOCK_DIM;
 
//     caculateWithMatrix<<<numOfBlocks, threadsPerBlock>>>(first_str, dev_s2, n2, dev_result ,off_set);
 
//     // copy the result back from the GPU to the CPU
//     int result=0;
//     cudaMemcpy(&result, dev_result, sizeof(int), cudaMemcpyDeviceToHost);		
	    
//     // free memory on the GPU side
//     cudaFree(dev_s2);
//     cudaFree(dev_result);

//     return result;
// }



// char *offsetFirstStr(int offset , int lenght)
// {
//     char *result, *returnStr;
//     cudaMalloc((void **)&result, lenght);
//     int threadsPerBlock = BLOCK_DIM;
//     int numOfBlocks = 1;
//     change_offset<<<numOfBlocks, threadsPerBlock>>>(result, offset);
//     returnStr = (char *)malloc(lenght * sizeof(char));
//     cudaMemcpy(returnStr, result, lenght * sizeof(char), cudaMemcpyDeviceToHost);
//     cudaFree(result);
//     return returnStr;
// }

__global__ void change_mutant_sequence(char *str, const char *str_to_change, int k, int size_str)
{
    int tid = threadIdx.x + blockIdx.x * blockDim.x;

    if (tid < size_str)
    {
        char c = gpu_toupper(str[tid]);
        if (c >= 'A' && c < 'Z')
        {
            if (tid >= k)
            {
                str[tid] = c + 1;
            }
        }
        else if (c == 'Z')
        {
            str[tid] = 'A';
        }

        // Use str_to_change for mutation
        str[tid] = str_to_change[tid];
    }
}

int Mutant_Sequence_cuda(int k, int size_str, const char *str_to_change , char** return_pointer)
{
    char *result, *returnStr;
    cudaMalloc((void **)&result, size_str * sizeof(char));
    cudaMalloc()
    int threadsPerBlock = BLOCK_DIM;
    int numOfBlocks = (size_str + threadsPerBlock - 1) / threadsPerBlock; // Calculate the number of blocks

    change_mutant_sequence<<<numOfBlocks, threadsPerBlock>>>(result, str_to_change, k, size_str);

    returnStr = (char *)malloc(size_str * sizeof(char));
    cudaMemcpy(returnStr, result, size_str * sizeof(char), cudaMemcpyDeviceToHost);
    cudaFree(result);
    return returnStr;
}
