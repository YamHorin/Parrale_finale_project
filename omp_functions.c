#include <omp.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>


#define MATRIX_SIZE 26
#define ROOT 0
#define MAX_STRING_SIZE 3000

void caculate_max_score_no_grade_table(char* str_to_check , char* first_str , struct score_alignment* AS_ptr)
{
    int lenght_first_str = strlen(first_str);
    int size_str_to_check = strlen(str_to_check);
    int sqn_taries = (size_str_to_check < lenght_first_str) ? (lenght_first_str - size_str_to_check)
                                                        : (size_str_to_check - lenght_first_str);
    int off_set, max_off_set, max_score = 0;
    int k, max_k, score;
    for (off_set = 0; off_set <= sqn_taries; off_set++)
    {
        for (k = 0; k < size_str_to_check; k++)
        {
            score = caculate_result_without_matrix( first_str, str_to_check,off_set, k);
            if (max_score <= score)
            {
                AS_ptr->K = k;
                AS_ptr->off_set = off_set;
                AS_ptr->score = score;
                max_score = score;
            }
        }
    }
    return 0;

}


void caculate_max_score_grade_table(char* str_to_check , char* first_str , int matrix[MATRIX_SIZE][MATRIX_SIZE] , struct score_alignment* AS_ptr)
{
    int lenght_first_str = strlen(first_str);
    int size_str_to_check = strlen(str_to_check);
    int sqn_taries = (size_str_to_check < lenght_first_str) ? (lenght_first_str - size_str_to_check)
                                                        : (size_str_to_check - lenght_first_str);
    int off_set, max_off_set, max_score = 0;
    int k, max_k, score;
    for (off_set = 0; off_set <= sqn_taries; off_set++)
    {
        for (k = 0; k < size_str_to_check; k++)
        {
            score = calculate_result_with_matrix( first_str, str_to_check, matrix, off_set, k);
            if (max_score <= score)
            {
                AS_ptr->k = k;
                AS_ptr->off_set = off_set;
                AS_ptr->score = score;
                max_score = score;
            }
        }
    }

}


int caculate_result_without_matrix(const char* first_str, const char *s2, int off_set, int k)
{
    int length = strlen(s2);
    int result = 0;
#pragma omp parallel for reduction(+ : result)
    for (int i = 0; i < length; i++)
    {
        if (i >= k)
            result = *((first_str + i) + off_set) == toupper(*((s2 + i)) + 1);
        else
        {
            if (*((first_str + i) + off_set) == s2[i])
                result++;
        }
    }
    return result;
}

int calculate_result_with_matrix(const char* first_str, const char *s2, int matrix[MATRIX_SIZE][MATRIX_SIZE], int off_set, int k)
{
    int length = strlen(s2);
    int result = 0;

#pragma omp parallel for reduction(+ : result)
    for (int i = 0; i < length; i++)
    {

        char c1 = *(first_str + i + off_set);
        char c2 = *(s2 + i);
        c1 = toupper(c1);
        c2 = toupper(c2);
        int x = c1 - 'A';
        int y = c2 - 'A'; 
        if (i >= k)
        {
            y++;
            if (c2 == 'Z')
                y = 0;
        }

        if (x >= 0 && x < MATRIX_SIZE && y >= 0 && y < MATRIX_SIZE) // Check bounds
        {
            result += matrix[x][y];
        }
    }
    return result;
}


/*
omp_functions.c: In function ‘void caculate_max_score_no_grade_table(char*, char*, score_alignment*)’:
omp_functions.c:23:21: error: ‘caculate_result_without_matrix’ was not declared in this scope
   23 |             score = caculate_result_without_matrix( first_str, str_to_check,off_set, k);
      |                     ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
omp_functions.c:26:23: error: invalid use of incomplete type ‘struct score_alignment’
   26 |                 AS_ptr->K = k;
      |                       ^~
omp_functions.c:11:86: note: forward declaration of ‘struct score_alignment’
   11 | void caculate_max_score_no_grade_table(char* str_to_check , char* first_str , struct score_alignment* AS_ptr)
      |                                                                                      ^~~~~~~~~~~~~~~
omp_functions.c:27:23: error: invalid use of incomplete type ‘struct score_alignment’
   27 |                 AS_ptr->off_set = off_set;
      |                       ^~
omp_functions.c:11:86: note: forward declaration of ‘struct score_alignment’
   11 | void caculate_max_score_no_grade_table(char* str_to_check , char* first_str , struct score_alignment* AS_ptr)
      |                                                                                      ^~~~~~~~~~~~~~~
omp_functions.c:28:23: error: invalid use of incomplete type ‘struct score_alignment’
   28 |                 AS_ptr->score = score;
      |                       ^~
omp_functions.c:11:86: note: forward declaration of ‘struct score_alignment’
   11 | void caculate_max_score_no_grade_table(char* str_to_check , char* first_str , struct score_alignment* AS_ptr)
      |                                                                                      ^~~~~~~~~~~~~~~
omp_functions.c:33:12: error: return-statement with a value, in function returning ‘void’ [-fpermissive]
   33 |     return 0;
      |            ^
omp_functions.c: In function ‘void caculate_max_score_grade_table(char*, char*, int (*)[26], score_alignment*)’:
omp_functions.c:50:21: error: ‘calculate_result_with_matrix’ was not declared in this scope
   50 |             score = calculate_result_with_matrix( first_str, str_to_check, matrix, off_set, k);
      |                     ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
omp_functions.c:53:23: error: invalid use of incomplete type ‘struct score_alignment’
   53 |                 AS_ptr->k = k;
      |                       ^~
omp_functions.c:11:86: note: forward declaration of ‘struct score_alignment’
   11 | void caculate_max_score_no_grade_table(char* str_to_check , char* first_str , struct score_alignment* AS_ptr)
      |                                                                                      ^~~~~~~~~~~~~~~
omp_functions.c:54:23: error: invalid use of incomplete type ‘struct score_alignment’
   54 |                 AS_ptr->off_set = off_set;
      |                       ^~
omp_functions.c:11:86: note: forward declaration of ‘struct score_alignment’
   11 | void caculate_max_score_no_grade_table(char* str_to_check , char* first_str , struct score_alignment* AS_ptr)
      |                                                                                      ^~~~~~~~~~~~~~~
omp_functions.c:55:23: error: invalid use of incomplete type ‘struct score_alignment’
   55 |                 AS_ptr->score = score;
      |                       ^~
omp_functions.c:11:86: note: forward declaration of ‘struct score_alignment’
   11 | void caculate_max_score_no_grade_table(char* str_to_check , char* first_str , struct score_alignment* AS_ptr)
*/