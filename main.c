#include <omp.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
#include "mpi.h"
#include "cFunctions.h"
#include "cudaFunctions.h"
#include "cudaFunctions2.h" //test

#define MATRIX_SIZE 26
#define ROOT 0
#define MAX_STRING_SIZE 3000

// enums

enum matrix_score
{
    THERE_IS_MATRIX_SCORE,
    NO_MATRIX_SCORE
} how_to_caculate;

enum tags
{
    WORK,
    STOP,
    DONE
};

// static values

int lenght_first_str;
int number_strings;
char *first_str;
int matrix[MATRIX_SIZE][MATRIX_SIZE];

void init(int argc, char **argv);
int caculate_result_without_matrix(const char *s2, int off_set);
int calculate_result_with_matrix(const char *s2, int matrix[MATRIX_SIZE][MATRIX_SIZE], int off_set);

int main(int argc, char *argv[])
{

    int my_rank, num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // making new type-struct score_alignment
    MPI_Datatype mpi_score_alignment_type;
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
    MPI_Type_create_struct(4, block_lengths, displacements, types, &mpi_score_alignment_type);
    MPI_Type_commit(&mpi_score_alignment_type);

    if (my_rank == 0)
    {
        init(argc, argv);
        int int_enum = (int)how_to_caculate;
        MPI_Bcast(&int_enum, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
        if (how_to_caculate == THERE_IS_MATRIX_SCORE)
            MPI_Bcast(matrix, MATRIX_SIZE * MATRIX_SIZE, MPI_INT, ROOT, MPI_COMM_WORLD);
        MPI_Bcast(&lenght_first_str, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
        MPI_Bcast(first_str, lenght_first_str * sizeof(char), MPI_CHAR, ROOT, MPI_COMM_WORLD);

        MPI_Status status;
        char *str_to_send;
        int str_length;
        int worker_rank;
#pragma omp parallel for private(str_to_send, worker_rank)
        for (worker_rank = 1; worker_rank < num_procs; worker_rank++)
        {

            str_to_send = createDynStr();
            str_length = strlen(str_to_send);
#ifdef DEBUG
            printf("send to rank %d -%s\n", worker_rank, str_to_send);
#endif
            MPI_Send(&str_length, 1, MPI_INT, worker_rank, WORK, MPI_COMM_WORLD);
            MPI_Send(str_to_send, (str_length + 1) * sizeof(char), MPI_CHAR, worker_rank, WORK, MPI_COMM_WORLD);
        }
        double t_start = MPI_Wtime();
        // test
        char *str_to_check = createDynStr();
        printf("%s\n", str_to_check);
        int score = caculate_cuda(str_to_check, first_str, matrix);
        if (score != 0)
        {
            printf("error in cuda");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }

        int str_send = num_procs;
        int tasks = number_strings;
        int tasks_done;
        struct score_alignment localMax;
        for (tasks_done = 0; tasks_done < number_strings - 1; tasks_done++)
        {
            localMax.score = 0;
            localMax.K = 0;
            localMax.off_set = 9;
            MPI_Recv(&localMax, 1, mpi_score_alignment_type, MPI_ANY_SOURCE,
                     DONE, MPI_COMM_WORLD, &status);
            printf("\nfor the string %s \n, we found that the max score alignment %d is from K  - %d and off set - %d  \n",
                   localMax.str, localMax.score, localMax.K, localMax.off_set);
            int tasks_not_sent_yet = tasks - str_send;
            if (tasks_not_sent_yet > 0)
            {

                str_to_send = createDynStr();
                str_length = strlen(str_to_send);
                MPI_Send(&str_length, 1, MPI_INT, status.MPI_SOURCE, WORK, MPI_COMM_WORLD);
                MPI_Send(str_to_send, (str_length + 1) * sizeof(char), MPI_CHAR, status.MPI_SOURCE, WORK, MPI_COMM_WORLD);
                str_send++;
            }
            else
            {
                /* send STOP message. message has no data */
                int dummy = 9;
                MPI_Send(&dummy, 1, MPI_INT, status.MPI_SOURCE,
                         STOP, MPI_COMM_WORLD);
            }
        }
        fprintf(stderr, "sequential time: %f secs\n", MPI_Wtime() - t_start);
    }
    else
    {
        int size_str_to_check, enumGet;
        char str_to_check[MAX_STRING_SIZE];
        MPI_Bcast(&enumGet, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

        how_to_caculate = (enum matrix_score)enumGet;
        if (how_to_caculate == THERE_IS_MATRIX_SCORE)
            MPI_Bcast(matrix, MATRIX_SIZE * MATRIX_SIZE, MPI_INT, ROOT, MPI_COMM_WORLD);

        MPI_Bcast(&lenght_first_str, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

        first_str = (char *)malloc(lenght_first_str * sizeof(char));

        MPI_Bcast(first_str, lenght_first_str * sizeof(char), MPI_CHAR, ROOT, MPI_COMM_WORLD);

        MPI_Status status;
        int tag, sqn_taries;
        do
        {

            MPI_Recv(&size_str_to_check, 1, MPI_INT,
                     ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            tag = status.MPI_TAG;
            struct score_alignment AS_max;
            if (tag == WORK)
            {
                MPI_Recv(str_to_check, (size_str_to_check + 1) * sizeof(char),
                         MPI_CHAR, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                AS_max.score = -1;
                sqn_taries = (size_str_to_check < lenght_first_str) ? (lenght_first_str - size_str_to_check)
                                                                    : (size_str_to_check - lenght_first_str);
                char str_k[MAX_STRING_SIZE];
                int off_set, max_off_set;
                int k, max_k, score;

                for (off_set = 0; off_set <= sqn_taries; off_set++)
                {
                    for (k = 0; k < size_str_to_check; k++)
                    {
                        strncpy(str_k, str_to_check, MAX_STRING_SIZE);

                        // test

                        Mutanat_Squence(str_k, k, size_str_to_check);
                        // int r = Mutanat_Squence_cuda(str_k, k, size_str_to_check);
                        // if (r != 0)
                        // {
                        //     printf("error in cuda");
                        //     MPI_Abort(MPI_COMM_WORLD, -1);
                        // }

                        if (how_to_caculate == NO_MATRIX_SCORE)
                            score = caculate_result_without_matrix(str_k, off_set);
                        else
                            score = calculate_result_with_matrix(str_k, matrix, off_set);

                        // printf("old str  - %s  str %s , <MS> = %d \nscore = %d\n", str_to_check, str_k ,k,score);

                        if (AS_max.score < score)
                        {

                            max_k = k;
                            max_off_set = off_set;
                            AS_max.score = score;
                        }
                    }
                    str_k[0] = '\0';
                }

                strncpy(AS_max.str, str_to_check, MAX_STRING_SIZE - 1);
                AS_max.str[MAX_STRING_SIZE] = '\0';
                AS_max.off_set = max_off_set;
                AS_max.K = max_k;
                MPI_Send(&AS_max, 1, mpi_score_alignment_type, ROOT, DONE, MPI_COMM_WORLD);
            }

        } while (tag != STOP);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    free(first_str);
    MPI_Type_free(&mpi_score_alignment_type);
    MPI_Finalize();
    return EXIT_SUCCESS;
}

void init(int argc, char **argv)
{
    first_str = createDynStr();
    if (first_str == NULL)
    {
        fprintf(stderr, "line 1 should be the first string\n");
        exit(1);
    }
    lenght_first_str = strlen(first_str);
    if (!(scanf("%d", &number_strings)))
    {
        fprintf(stderr, "line 2 should be the numebr of strings to check\n");
        exit(1);
    }
    how_to_caculate = NO_MATRIX_SCORE;
    if (argc == 2)
    {
        how_to_caculate = THERE_IS_MATRIX_SCORE;
        if (readMatrixFromFile(argv[1], matrix) == 0)
            printf("Matrix read successfully!\n");
        else
            printf("There was an error reading the matrix.\n");
    }
}

int caculate_result_without_matrix(const char *s2, int off_set)
{
    int length = strlen(s2);
    int result = 0;
#pragma omp parallel for reduction(+ : result)
    for (int i = 0; i < length; i++)
    {
        if (*((first_str + i) + off_set) == s2[i])
            result++;
    }
    return result;
}

int calculate_result_with_matrix(const char *s2, int matrix[MATRIX_SIZE][MATRIX_SIZE], int off_set)
{
    int length = strlen(s2);
    int result = 0;
#pragma omp parallel for reduction(+ : result)
    for (int i = 0; i < length; i++)
    {
        int x = *(first_str + i + off_set) - 'A';
        int y = s2[i] - 'A';
        result += matrix[x][y];
    }

    return result;
}