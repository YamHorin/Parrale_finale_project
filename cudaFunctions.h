#ifndef EXE_C
#define EXE_C

#define MATRIX_SIZE 26
#define MAX_STRING_SIZE 3000


int device_strlen(const char* str);
void device_strncpy(char* dest, const char* src, int n);
char gpu_toupper(char c);
int caculate_cuda(const char *str_to_check, const char *first_str, int matrix[MATRIX_SIZE][MATRIX_SIZE] , int my_rank);
void caculate_result(char *str_to_check, char *first_str, int size_second_str,  int *result , int off_set, int* matrix ,int k);
void caculate_result_without_matrix(char *str_to_check, char *first_str, int size_second_str, int *result, int off_set,int k);
int caculate_cuda_without_matrix(const char *str_to_check, const char *first_str, int my_rank) ;
#endif