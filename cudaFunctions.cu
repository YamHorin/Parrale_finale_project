#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <cuda_runtime.h> // Include CUDA runtime header

#define BLOCK_DIM 1024 // number of threads in a block
#define MAX_STRING_SIZE 3000
#define MATRIX_SIZE 26

__device__ char gpu_toupper(char c)
{
    if (c >= 'a' && c <= 'A')
        return c - ('a' - 'A');
    return c;
}

__global__ void Mutanat_SquenceKernel(char *str, int k, int size_str)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= k && tid <= size_str)
    {
        if (gpu_toupper(str[tid]) >= 'Z')
        {
            str[tid] = 'A';
        }
        if (tid == size_str)
        {
            str[tid] = '\0';
        }
        else
        {
            str[tid] = gpu_toupper(str[tid] + 1);
        }
    }
}

int Mutanat_Squence_cuda(char *str, int k, int size_str)
{
    char *d_str;
    int strSize = size_str + 1; // Include space for '\0'

    // Allocate memory on the GPU
    cudaError_t cudaStatus = cudaMalloc((void **)&d_str, strSize * sizeof(char));
    if (cudaStatus != cudaSuccess)
    {
        fprintf(stderr, "cudaMalloc failed: %s\n", cudaGetErrorString(cudaStatus));
        return -1;
    }

    // Copy the input string to the GPU
    cudaStatus = cudaMemcpy(d_str, str, strSize * sizeof(char), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess)
    {
        fprintf(stderr, "cudaMemcpy failed: %s\n", cudaGetErrorString(cudaStatus));
        cudaFree(d_str);
        return -1;
    }

    // Calculate the number of threads per block and the number of blocks
    int threadsPerBlock = 256;
    int numBlocks = (strSize + threadsPerBlock - 1) / threadsPerBlock;

    // Launch the kernel
    Mutanat_SquenceKernel<<<numBlocks, threadsPerBlock>>>(d_str, k, size_str);

    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess)
    {
        fprintf(stderr, "kernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        cudaFree(d_str);
        return -1;
    }

    // Copy the result back from the GPU to the CPU
    cudaStatus = cudaMemcpy(str, d_str, strSize * sizeof(char), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess)
    {
        fprintf(stderr, "cudaMemcpy (device to host) failed: %s\n", cudaGetErrorString(cudaStatus));
        cudaFree(d_str);
        return -1;
    }

    // Free the GPU memory
    cudaFree(d_str);

    return 0; // Return 0 on success
}
