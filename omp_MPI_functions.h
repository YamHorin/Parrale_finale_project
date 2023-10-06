#ifndef EXE_I
#define EXE_I
#include "mpi.h"
#define MATRIX_SIZE 26
#define ROOT 0
#define MAX_STRING_SIZE 3000


void caculate_max_score_no_grade_table(char* str_to_check , char* first_str , struct score_alignment* AS_ptr);
void caculate_max_score_grade_table(char* str_to_check , char* first_str , int matrix[MATRIX_SIZE][MATRIX_SIZE], struct score_alignment* AS_ptr);
void make_datatype(MPI_Datatype* mpi_score_alignment_type);


#endif
