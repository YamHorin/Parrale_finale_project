#ifndef EXE_P
#define EXE_P


#define MATRIX_SIZE 26

char gpu_toupper(char c);
int getScoreFromMatrix(char a, char b);
void scan_plus(int *array, int size);
void caculate(const char  *s1, const char *s2, int n2,  int *result , int off_set);
void caculateWithMatrix(const char  *s1, const char *s2, int n2,  int *result , int off_set);
int computeOnGPU(const char *s2 , int off_set);
int computeOnGPUWithMatrix( const char *s2 ,const int matrix[MATRIX_SIZE][MATRIX_SIZE] , int off_set);
char* Mutanat_Squence_cuda(int k , int size_str);
char *offsetFirstStr(int offset , int lenght);
void change_mutant_squence(char *str, int k , int size_str);

#endif