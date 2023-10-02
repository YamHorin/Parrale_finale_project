#ifndef EXE_P
#define EXE_P


#define MATRIX_SIZE 26
#define MAX_STRING_SIZE 3000
// char *offsetFirstStr(int offset , int lenght);

// int getScoreFromMatrix(char a, char b);
// void scan_plus(int *array, int size);
// void caculate(const char  *s1, const char *s2, int n2,  int *result , int off_set);
// void caculateWithMatrix(const char  *s1, const char *s2, int n2,  int *result , int off_set);
// int computeOnGPU(const char *s2 , int off_set);
// int computeOnGPUWithMatrix( const char *s2 ,const int matrix[MATRIX_SIZE][MATRIX_SIZE] , int off_set);

char gpu_toupper(char c);

int Mutant_Sequence_cuda(int k, int size_str, const char *str_to_change, char returnStr[MAX_STRING_SIZE]);

void change_mutant_squence(char *str, int k , int size_str);

#endif
