#include <omp.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
#include "mpi.h"
#include "cFunctions.h"
#include "cudaFunctions.h"
#include "main.h"

int lenght_first_str;
int number_strings;
char *first_str;
int matrix[MATRIX_SIZE][MATRIX_SIZE];

int main(int argc, char *argv[])
{
    clock_t t_program = clock(), t_omp, t_cuda;
    int my_rank, num_procs;
    double time_taken;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (my_rank == 0)
    {
        printf("this is a sequential run\n");
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

        int chunk_size = number_strings > num_procs? number_strings/num_procs : num_procs/number_strings;
        if (number_strings % num_procs)
            chunk_size += number_strings > num_procs? number_strings%num_procs : num_procs%number_strings;
        char *strings_to_check[chunk_size];
        for (int i = 0; i < chunk_size; i++)
        {
            strings_to_check[i] = createDynStr();
        }
        chunk_size = number_strings > num_procs? number_strings/num_procs : num_procs/number_strings;
        for (worker_rank = 1; worker_rank < num_procs; worker_rank++)
        {

            MPI_Send(&chunk_size, 1, MPI_INT, worker_rank, GET, MPI_COMM_WORLD);
            int start = (chunk_size * worker_rank);
            int finish = (chunk_size * worker_rank) + chunk_size;
            for (int i = start; i < finish; i++)
            {
                str_to_send = createDynStr();
                str_length = strlen(str_to_send);
                MPI_Send(&str_length, 1, MPI_INT, worker_rank, WORK, MPI_COMM_WORLD);
                MPI_Send(str_to_send, (str_length + 1) * sizeof(char), MPI_CHAR, worker_rank, WORK, MPI_COMM_WORLD);
            }
        }
        t_cuda = clock();

        for (int i = 0; i < chunk_size; i++)
        {
            // caculate_cuda
            char *str_to_check = strings_to_check[i];
            int score;
            if (how_to_caculate == NO_MATRIX_SCORE)
                score = caculate_cuda_without_matrix(str_to_check, first_str, my_rank);
            else
                score = caculate_cuda(str_to_check, first_str, matrix, my_rank);
        }
        t_cuda = clock() - t_cuda;
        time_taken = ((double) t_cuda) / CLOCKS_PER_SEC; // in seconds
        fprintf(stderr,"\nCUDA part took %.2f seconds to execute\n", time_taken);
        //struct score_alignment scores[chunk_size];
        for (worker_rank = 1; worker_rank < num_procs; worker_rank++)
        {
            int dummy = 0;
            MPI_Sendrecv(&dummy, 1, MPI_INT, worker_rank, PRINT, &dummy, 1, MPI_INT, worker_rank, DONE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            /* send STOP message. message has no data */
            MPI_Send(&dummy, 1, MPI_INT, worker_rank,
                     STOP, MPI_COMM_WORLD);
        }

        t_program = clock() - t_program;
		time_taken = ((double) t_program) / CLOCKS_PER_SEC; // in seconds
		fprintf(stderr,"\nProgram took %.2f seconds to execute\n", time_taken);
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
        int chunk_size;
        MPI_Recv(&chunk_size, 1, MPI_INT,
                 ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        char strings[chunk_size][MAX_STRING_SIZE];
        tag = status.MPI_TAG;
        struct score_alignment AS_max;
        struct score_alignment scores[chunk_size];
        do
        {
            if (tag == PRINT)
            {
                for (int i = 0; i < chunk_size; i++)
                {
                    printf("\nmy_rank [%d] for the string %s \nWe found that the max score alignment %d is from K  - %d and off set - %d  \n",
                           my_rank, scores[i].str, scores[i].score, scores[i].K, scores[i].off_set);
                }
                int dummy;
                MPI_Send(&dummy, 1, MPI_INT, ROOT, DONE, MPI_COMM_WORLD);
            }

            if (tag == GET)
            {
                for (int i = 0; i < chunk_size; i++)
                {
                    MPI_Recv(&size_str_to_check, 1, MPI_INT,
                             ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    MPI_Recv(strings[i], (size_str_to_check + 1) * sizeof(char),
                             MPI_CHAR, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                tag = status.MPI_TAG;
            }
            if (tag == WORK)
            {
                for (int i = 0; i < chunk_size; i++)
                {

                    size_str_to_check = strlen(strings[i]);
                    AS_max.score = -1;
                    sqn_taries = (size_str_to_check < lenght_first_str) ? (lenght_first_str - size_str_to_check)
                                                                        : (size_str_to_check - lenght_first_str);
                    int off_set, max_off_set, max_score = 0;
                    int k, max_k, score;
                    t_omp = clock();
                    for (off_set = 0; off_set <= sqn_taries; off_set++)
                    {
                        for (k = 0; k < size_str_to_check; k++)
                        {

                            if (how_to_caculate == NO_MATRIX_SCORE)
                                score = caculate_result_without_matrix(strings[i], off_set, k);
                            else
                                score = calculate_result_with_matrix(strings[i], matrix, off_set, k);
                            if (max_score <= score)
                            {
                                max_k = k;
                                max_off_set = off_set;
                                scores[i].score = score;
                                max_score = score;
                            }
                        }
                    }
                    t_omp = clock()-t_omp;
                    time_taken = ((double) t_omp) / CLOCKS_PER_SEC; // in seconds
	                fprintf(stderr,"\nOpenMP part took %.2f seconds to execute\n", time_taken);

                    strncpy(scores[i].str, strings[i], MAX_STRING_SIZE - 1);
                    scores[i].score = max_score;
                    scores[i].str[MAX_STRING_SIZE] = '\0';
                    scores[i].off_set = max_off_set;
                    scores[i].K = max_k;
                }
            }
            MPI_Recv(&size_str_to_check, 1, MPI_INT,
                     ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            tag = status.MPI_TAG;

        } while (tag != STOP);
    }

    free(first_str);

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

int caculate_result_without_matrix(const char *s2, int off_set, int k)
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

int calculate_result_with_matrix(const char *s2, int matrix[MATRIX_SIZE][MATRIX_SIZE], int off_set, int k)
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