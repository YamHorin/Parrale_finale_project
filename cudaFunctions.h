#ifndef EXE_C
#define EXE_C

#define MATRIX_SIZE 26
#define MAX_STRING_SIZE 3000
struct score_alignment {
    int score;
    int K;
    int off_set;
    char str[MAX_STRING_SIZE];
};
int device_strlen(const char* str);
void device_strncpy(char* dest, const char* src, int n);
char gpu_toupper(char c);
int caculate_cuda(const char *str_to_check, const char *first_str, int matrix[MATRIX_SIZE][MATRIX_SIZE]);
void caculate_result(char *str_to_check, char *first_str, int size_second_str,  int *result , int off_set, int* matrix ,int k);

#endif