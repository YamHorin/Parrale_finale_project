#include <stdio.h>
#include <cstring>
#include <ctype.h>
#include <string.h>
#include <cuda_runtime.h> 

const int MAX_STRING_SIZE = 3000; // Define MAX_STRING_SIZE as needed
const int MATRIX_SIZE = 26;       // Define MATRIX_SIZE as needed

struct score_alignment {
    int score;
    int K;
    int off_set;
    char str[MAX_STRING_SIZE];
};
__device__ int device_strlen(const char* str) {
    int length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

__device__ void device_strncpy(char* dest, const char* src, int n) {
    for (int i = 0; i < n; i++) {
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

__device__ int caculate_result_without_matrix(const char *s2, int off_set, const char *first_str) {
    int length = device_strlen(s2);
    int result = 0;
    
    for (int i = 0; i < length; i++) {
        if (first_str[i + off_set] == s2[i]) {
            result++;
        }
    }
    
    return result;
}

__device__ int calculate_result_with_matrix(const char *s2, int *matrix, int off_set, const char *first_str) {
    int length = device_strlen(s2);
    int result = 0;
    
    for (int i = 0; i < length; i++) {
        int x = first_str[i + off_set] - 'A';
        int y = s2[i] - 'A';
        
        if (x < 0 || x >= MATRIX_SIZE || y < 0 || y >= MATRIX_SIZE) {
            // Handle out-of-bounds characters.
            return -1; // or any appropriate error code
        }
        result += matrix[x * MATRIX_SIZE + y];
    }
    
    return result;
}

__device__ void Mutanat_Squence2(char *str, int k, int size_str) {
    for (int i = k; i <= size_str; i++) {
        if (gpu_toupper(str[i]) >= 'Z') {
            str[i] = 'A';
        }
        if (i == size_str) {
            str[i] = '\0';
        } else {
            str[i] = gpu_toupper(str[i] + 1);
        }
    }
}
__global__ void caculate_result(char *str_to_check, char *first_str, int size_second_str,  int *result , int off_set
                                , int* matrix ,int k)
{
     __shared__ int r;
     int value;
     int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if(tid == 0) r = 0;
    __syncthreads();
    if (tid <size_second_str)
    {
        int i  = tid;
        if (tid>=k)
        {
            
            int x = gpu_toupper(first_str[i + off_set]) - 'A';
            int y = (gpu_toupper(str_to_check[i])+1) - 'A';
            value = matrix[x * MATRIX_SIZE + y];
        }
        else
        {
            int x = gpu_toupper(first_str[i + off_set]) - 'A';
            int y = (gpu_toupper(str_to_check[i])) - 'A';
            value = matrix[x * MATRIX_SIZE + y];
        }
        atomicAdd(&r , value);

    }
     __syncthreads();
     if (tid == 0)
        *result = r;

}
// __global__ void cuda_caculate_max_score(char *str_to_check, char *first_str, int how_to_caculate,
//                                         int *matrix, score_alignment *localMax) {
//     int tid = blockIdx.x * blockDim.x + threadIdx.x;
    
//     if (tid < sqn_taries * size_str_to_check) {
//         int off_set = tid / size_str_to_check;
//         int k = tid % size_str_to_check;

//         // Create a copy of str_to_check
//         char mutated_str[MAX_STRING_SIZE];
//         device_strncpy(mutated_str, str_to_check, MAX_STRING_SIZE - 1);
//         mutated_str[MAX_STRING_SIZE - 1] = '\0';

//         // Mutate the sequence
//         Mutanat_Squence2(mutated_str, k, size_str_to_check);

//         int score = 0;

//         if (how_to_caculate == 0) {
//             score = caculate_result_without_matrix(mutated_str, off_set, first_str);
//         } else {
//             score = calculate_result_with_matrix(mutated_str, matrix, off_set, first_str);
//         }

//         // Update localMax if a higher score is found
//         atomicMax(&localMax->score, score);
//         if (score == localMax->score) {
//             localMax->K = k;
//             localMax->off_set = off_set;
//         }
//     }
// }

int caculate_cuda(const char *str_to_check, const char *first_str, int matrix[MATRIX_SIZE][MATRIX_SIZE]) {


    // Calculate the lengths of the strings
    int size_str_to_check = strlen(str_to_check);
    int size_first_str = strlen(first_str);

    // Check if the lengths exceed the maximum allowed size
    if (size_str_to_check >= MAX_STRING_SIZE || size_first_str >= MAX_STRING_SIZE) {
        printf("Error: String length exceeds MAX_STRING_SIZE.\n");
        return -1; // or any appropriate error code
    }

    // Allocate memory on the GPU
    struct score_alignment localMax;
    char *d_str_to_check, *d_first_str;
    int *d_matrix , *dev_result;
    cudaMalloc((void**)&d_str_to_check, MAX_STRING_SIZE);
    cudaMalloc((void**)&d_first_str, MAX_STRING_SIZE);
    cudaMalloc((void**)&d_matrix, MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    cudaMalloc((void**)&dev_result, sizeof(int));
    // Copy data from host to device
    cudaMemcpy(d_str_to_check, str_to_check, size_str_to_check + 1, cudaMemcpyHostToDevice);
    cudaMemcpy(d_first_str, first_str, size_first_str + 1, cudaMemcpyHostToDevice);
    cudaMemcpy(d_matrix, matrix, MATRIX_SIZE * MATRIX_SIZE * sizeof(int), cudaMemcpyHostToDevice);
    int threadsPerBlock = 256;
    int result=0;
    int blocksPerGrid = (size_str_to_check > MAX_STRING_SIZE)? size_str_to_check/threadsPerBlock : 1;

    int max_score = 0;
    int sqn_taries = (size_str_to_check < size_first_str) ? (size_first_str - size_str_to_check) : (size_str_to_check - size_first_str);

    for (int off_set = 0; off_set <= sqn_taries; off_set++)
    {
        for (int k = 0; k < size_str_to_check; k++)
        {
               caculate_result <<<blocksPerGrid, threadsPerBlock>>>(
                d_str_to_check,d_first_str,  size_str_to_check,  
                dev_result,  off_set,  d_matrix , k);
            cudaMemcpy(&result, dev_result, sizeof(int), cudaMemcpyDeviceToHost);
            if (result >max_score)
            {
                max_score = result;
                localMax.score = result;
                localMax.K = k;
                localMax.off_set =  off_set;
            }


        }
    }
    // Define block and grid dimensions
    

    // Launch the CUDA kernel

    // Copy the result back from the device to the host

    // Free allocated memory on the device
    cudaFree(d_str_to_check);
    cudaFree(d_first_str);
    cudaFree(d_matrix);
    cudaFree(dev_result);
    // Print the result
    printf("\nFor the string %s,\n", str_to_check);
    printf("We found that the max score alignment %d is from K - %d and off set - %d\n", localMax.score, localMax.K, localMax.off_set);

    return localMax.score;
}

/*
for the string �Ss[� 
, we found that the max score alignment 22069 is from K  - 5 and off set - 0 

*/