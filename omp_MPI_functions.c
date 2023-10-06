#include <omp.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
#include "mpi.h"
#include "struct.h"

#define MATRIX_SIZE 26
#define ROOT 0
#define MAX_STRING_SIZE 3000



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

//to fix
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

            if (max_score <= score)
            {
                AS_ptr->K = k;
                AS_ptr->off_set = off_set;
                AS_ptr->score = score;
                max_score = score;
            }
        }
    }
}


void caculate_max_score_grade_table(char* str_to_check , char* first_str , int matrix[MATRIX_SIZE][MATRIX_SIZE] , struct score_alignment* AS_ptr)
{
    int lenght_first_str = strlen(first_str);
    int size_str_to_check = strlen(str_to_check);
    int sqn_taries = (size_str_to_check < lenght_first_str) ? (lenght_first_str - size_str_to_check)
                                                        : (size_str_to_check - lenght_first_str);
    int off_set, max_off_set, max_score = 0;
    int k, max_k, score;

    #pragma omp parallel private(k ,off_set ,max_score)
    for (off_set = 0; off_set <= sqn_taries; off_set++)
    {

        for (k = 0; k < size_str_to_check; k++)
        {
                #pragma omp for reduction(+ : score)
                for (int i = 0; i < size_str_to_check; i++)
                {

                    char c1 = *(first_str + i + off_set);
                    char c2 = *(str_to_check + i);
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
                        score += matrix[x][y];
                    }
                }
            #pragma omp critical
            if (max_score <= score)
            {
                AS_ptr->K = k;
                AS_ptr->off_set = off_set;
                AS_ptr->score = score;
                max_score = score;
            }
        }
    }

}

void make_datatype(MPI_Datatype* mpi_score_alignment_type)
{
    // making new type-struct score_alignment
    MPI_Datatype types[4] = {MPI_CHAR, MPI_INT, MPI_INT, MPI_INT};
    int block_lengths[4] = {MAX_STRING_SIZE, 1, 1, 1}; // Initialized to zeros
    MPI_Aint displacements[4];
    struct score_alignment temp; // Used to calculate displacements

    // Calculate displacements
    MPI_Get_address(&temp.str, &displacements[0]);
    MPI_Get_address(&temp.off_set, &displacements[1]);
    MPI_Get_address(&temp.K, &displacements[2]);
    MPI_Get_address(&temp.score, &displacements[3]);

    for (int i = 3; i > 0; i--)
    {
        displacements[i] -= displacements[0];
    }
    displacements[0] = 0;


    // Create the custom data type
    MPI_Type_create_struct(4, block_lengths, displacements, types, mpi_score_alignment_type);
    MPI_Type_commit(mpi_score_alignment_type);

}

