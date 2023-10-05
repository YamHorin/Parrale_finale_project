#ifndef EXE_I
#define EXE_I


#include "cudaFunctions.h"
void caculate_max_score_no_grade_table(char* str_to_check , char* first_str , struct score_alignment* AS_ptr);
void caculate_max_score_grade_table(char* str_to_check , char* first_str , int matrix[MATRIX_SIZE][MATRIX_SIZE], struct score_alignment* AS_ptr);
int caculate_result_without_matrix(const char* first_str, const char *s2, int off_set, int k);
int calculate_result_with_matrix(const char* first_str, const char *s2, int matrix[MATRIX_SIZE][MATRIX_SIZE], int off_set, int k);


#endif