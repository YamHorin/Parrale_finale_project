#ifndef EXE_C
#define EXE_C

#define MATRIX_SIZE 26
#define MAX_STRING_SIZE 3000

int caculate_result_without_matrix(const char *s2, int off_set, const char *first_str) ;
int calculate_result_with_matrix(const char *s2, int *matrix, int off_set, const char *first_str);
void Mutanat_Squence(char *str, int k, int size_str);
void cuda_caculate_max_score(char *str_to_check, char *first_str, int how_to_caculate,
                                        int *matrix, score_alignment *localMax);
int caculate_cuda(char *str_to_check, char *first_str ,int matrix[MATRIX_SIZE][MATRIX_SIZE]);


#endif