#include <stdio.h>
#include <cstring>
#include <ctype.h>
#include <string.h>
#include <cuda_runtime.h>
#include "struct.h"


__device__ int device_strlen(const char *str)
{
    int length = 0;
    while (str[length] != '\0')
    {
        length++;
    }
    return length;
}

__device__ void device_strncpy(char *dest, const char *src, int n)
{
    for (int i = 0; i < n; i++)
    {
        dest[i] = src[i];
    }
    dest[n] = '\0';
}

__device__ char gpu_toupper(char c)
{
    if (c >= 'a' && c <= 'A')
        return c - ('a' - 'A');
    return c;
}
__global__ void caculate_result_without_matrix(char *str_to_check, char *first_str, int size_second_str, int *result, int off_set, int k)
{
    __shared__ int r;
    int value;
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid == 0)
        r = 0;
    __syncthreads();
    if (tid < size_second_str)
    {
        int i = tid;
        if (tid >= k)
        {

            int x = gpu_toupper(first_str[i + off_set]) - 'A';
            int y = (gpu_toupper(str_to_check[i]) + 1) - 'A';
            value = (x == y);
        }
        else
        {
            int x = gpu_toupper(first_str[i + off_set]) - 'A';
            int y = (gpu_toupper(str_to_check[i])) - 'A';
            value = (x == y);
        }
        atomicAdd(&r, value);
    }
    __syncthreads();
    if (tid == 0)
        *result = r;
}
__global__ void caculate_result(char *str_to_check, char *first_str, int size_second_str, int *result, int off_set, int *matrix, int k)
{
    __shared__ int r;
    int value;
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid == 0)
        r = 0;
    __syncthreads();
    if (tid < size_second_str)
    {
        int i = tid;
        if (tid >= k)
        {

            int x = gpu_toupper(first_str[i + off_set]) - 'A';
            int y = (gpu_toupper(str_to_check[i]) + 1) - 'A';
            value = matrix[x * MATRIX_SIZE + y];
        }
        else
        {
            int x = gpu_toupper(first_str[i + off_set]) - 'A';
            int y = (gpu_toupper(str_to_check[i])) - 'A';
            value = matrix[x * MATRIX_SIZE + y];
        }
        atomicAdd(&r, value);
    }
    __syncthreads();
    if (tid == 0)
        *result = r;
}

int caculate_cuda(const char *str_to_check, const char *first_str, int matrix[MATRIX_SIZE][MATRIX_SIZE] , int my_rank)
{

    // Calculate the lengths of the strings
    int size_str_to_check = strlen(str_to_check);
    int size_first_str = strlen(first_str);

    // Check if the lengths exceed the maximum allowed size
    if (size_str_to_check >= MAX_STRING_SIZE || size_first_str >= MAX_STRING_SIZE)
    {
        printf("Error: String length exceeds MAX_STRING_SIZE.\n");
        return -1; // or any appropriate error code
    }

    // Allocate memory on the GPU
    struct score_alignment localMax;
    char *d_str_to_check, *d_first_str;
    int *d_matrix, *dev_result;

    cudaError_t err1 = cudaMalloc((void **)&d_str_to_check, MAX_STRING_SIZE);

    cudaError_t err2 = cudaMalloc((void **)&d_first_str, MAX_STRING_SIZE);

    cudaError_t err3 = cudaMalloc((void **)&d_matrix, MATRIX_SIZE * MATRIX_SIZE * sizeof(int));

    cudaError_t err4 = cudaMalloc((void **)&dev_result, sizeof(int));

    if (err1 != cudaSuccess || err2 != cudaSuccess || err3 != cudaSuccess || err4 != cudaSuccess)
    {
        fprintf(stderr, "CUDA  malloc error\n");
        exit(1);
    }
    // Copy data from host to device
    err1 = cudaMemcpy(d_str_to_check, str_to_check, size_str_to_check + 1, cudaMemcpyHostToDevice);

    err2 = cudaMemcpy(d_first_str, first_str, size_first_str + 1, cudaMemcpyHostToDevice);

    err3 = cudaMemcpy(d_matrix, matrix, MATRIX_SIZE * MATRIX_SIZE * sizeof(int), cudaMemcpyHostToDevice);
    if (err1 != cudaSuccess || err2 != cudaSuccess || err3 != cudaSuccess)
    {
        fprintf(stderr, "CUDA  memcpy 1-3 error\n");
        exit(1);
    }

    int threadsPerBlock = 256;
    int result = 0;
    int blocksPerGrid = (size_str_to_check > MAX_STRING_SIZE) ? size_str_to_check / threadsPerBlock : 1;
    int max_score = 0;
    int sqn_taries = (size_str_to_check < size_first_str) ? (size_first_str - size_str_to_check) : (size_str_to_check - size_first_str);

    for (int off_set = 0; off_set <= sqn_taries; off_set++)
    {
        for (int k = 0; k < size_str_to_check; k++)
        {
            caculate_result<<<blocksPerGrid, threadsPerBlock>>>(
                d_str_to_check, d_first_str, size_str_to_check,
                dev_result, off_set, d_matrix, k);
            err4 = cudaMemcpy(&result, dev_result, sizeof(int), cudaMemcpyDeviceToHost);
            if (err4 != cudaSuccess)
            {
                fprintf(stderr, "CUDA  memcpy 4 error\n");
                exit(1);
            }
            if (result >= max_score)
            {
                max_score = result;
                localMax.score = result;
                localMax.K = k;
                localMax.off_set = off_set;
            }
        }
    }

    cudaFree(d_str_to_check);
    cudaFree(d_first_str);
    cudaFree(d_matrix);
    cudaFree(dev_result);
    // Print the result
    printf("We found that the max score alignment %d is from K - %d and off set - %d\n", localMax.score, localMax.K, localMax.off_set);

    return 0;
}

int caculate_cuda_without_matrix(const char *str_to_check, const char *first_str , int my_rank)
{
    // Calculate the lengths of the strings
    int size_str_to_check = strlen(str_to_check);
    int size_first_str = strlen(first_str);

    // Check if the lengths exceed the maximum allowed size
    if (size_str_to_check >= MAX_STRING_SIZE || size_first_str >= MAX_STRING_SIZE)
    {
        printf("Error: String length exceeds MAX_STRING_SIZE.\n");
        return -1; // or any appropriate error code
    }

    // Allocate memory on the GPU
    struct score_alignment localMax;
    char *d_str_to_check, *d_first_str;
    int *dev_result;
    cudaError_t err1 = cudaMalloc((void **)&d_str_to_check, MAX_STRING_SIZE);

    cudaError_t err2 = cudaMalloc((void **)&d_first_str, MAX_STRING_SIZE);

    cudaError_t err3 = cudaMalloc((void **)&dev_result, sizeof(int));

    if (err1 != cudaSuccess || err2 != cudaSuccess || err3 != cudaSuccess)
    {
        fprintf(stderr, "CUDA  malloc  error\n");
        exit(1);
    }
    // Copy data from host to device
    err1 = cudaMemcpy(d_str_to_check, str_to_check, size_str_to_check + 1, cudaMemcpyHostToDevice);

    err2 = cudaMemcpy(d_first_str, first_str, size_first_str + 1, cudaMemcpyHostToDevice);

    if (err1 != cudaSuccess || err2 != cudaSuccess)
    {
        fprintf(stderr, "CUDA   memcpy  error\n");
        exit(1);
    }
    int threadsPerBlock = 256;
    int result = 0;
    int blocksPerGrid = (size_str_to_check > MAX_STRING_SIZE) ? size_str_to_check / threadsPerBlock : 1;
    int max_score = 0;
    int sqn_taries = (size_str_to_check < size_first_str) ? (size_first_str - size_str_to_check) : (size_str_to_check - size_first_str);

    for (int off_set = 0; off_set <= sqn_taries; off_set++)
    {
        for (int k = 0; k < size_str_to_check; k++)
        {
            caculate_result_without_matrix<<<blocksPerGrid, threadsPerBlock>>>(
                d_str_to_check, d_first_str, size_str_to_check,
                dev_result, off_set, k);
            err3 = cudaMemcpy(&result, dev_result, sizeof(int), cudaMemcpyDeviceToHost);
            if (err3 != cudaSuccess)
            {
                fprintf(stderr, "CUDA  memcpy 2 error\n");
                exit(1);
            }
            if (result >= max_score)
            {
                max_score = result;
                localMax.score = result;
                localMax.K = k;
                localMax.off_set = off_set;
            }
        }
    }

    cudaFree(d_str_to_check);
    cudaFree(d_first_str);
    cudaFree(dev_result);
    // Print the result
    printf("We found that the max score alignment %d is from K - %d and off set - %d\n", localMax.score, localMax.K, localMax.off_set);

    return 0;
}
