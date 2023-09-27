#define MAX_STRING_SIZE 3000

__device__ char gpu_toupper(char c);

__device__ int getScoreFromMatrix(char a, char b);

__device__ void scan_plus(int *array, int size);

__global__ void change_offset(char *str, int k , int size_str);
__global__ void change_offset(char *str, int offset);
__global__ void caculate(const char  *s1, const char *s2, int n2,  int *result , int off_set);
__global__ void caculateWithMatrix(const char  *s1, const char *s2, int n2,  int *result , int off_set);
void getFirstStr(char *s1, int n1);
void getStrToCheck(char *s1, int n1);
int computeOnGPU(const char *s2 , int off_set);
int computeOnGPUWithMatrix( const char *s2 ,const int matrix[MATRIX_SIZE][MATRIX_SIZE] , int off_set);
char* Mutanat_Squence_cuda(int k , int size_str);
char *offsetFirstStr(int offset , int lenght);