#include <omp.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
#include "mpi.h"
#include "cFunctions.h"
#include "main.h"
#include "cudaFunctions.h"
#include "omp_MPI_functions.h"
#include "struct.h"

int lenght_first_str;
int number_strings;
char *first_str;
int matrix[MATRIX_SIZE][MATRIX_SIZE];

int main(int argc, char *argv[])
{
    clock_t t_program, t_omp, t_cuda;
    int my_rank, num_procs;
    double time_taken;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Datatype mpi_score_alignment_type;
    make_datatype(&mpi_score_alignment_type);
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

        char *str_to_send;
        int str_length;
        int worker_rank;
        int master_chunk_size, chunk_size;
        master_chunk_size = number_strings >= num_procs ? number_strings / num_procs : num_procs / number_strings;
        if ((number_strings % num_procs) != 0)
            master_chunk_size += number_strings >= num_procs ? number_strings % num_procs : num_procs % number_strings;
        char *strings_to_check[master_chunk_size];
        for (int i = 0; i < master_chunk_size; i++)
        {
            strings_to_check[i] = createDynStr();
        }
        chunk_size = (number_strings - master_chunk_size) >= (num_procs - 1) ? (number_strings - master_chunk_size) / (num_procs - 1) : (num_procs - 1) / (number_strings - master_chunk_size);
        for (worker_rank = 1; worker_rank < num_procs; worker_rank++)
        {

            MPI_Send(&chunk_size, 1, MPI_INT, worker_rank, GET, MPI_COMM_WORLD);
            for (int i = 0; i < chunk_size; i++)
            {
                str_to_send = createDynStr();
                str_length = strlen(str_to_send);
                MPI_Send(&str_length, 1, MPI_INT, worker_rank, WORK, MPI_COMM_WORLD);
                MPI_Send(str_to_send, (str_length + 1) * sizeof(char), MPI_CHAR, worker_rank, WORK, MPI_COMM_WORLD);
            }
        }
        t_program = clock();
        t_cuda = clock();

        for (int i = 0; i < master_chunk_size; i++)
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
        time_taken = ((double)t_cuda) / CLOCKS_PER_SEC; // in seconds
        fprintf(stderr, "\nCUDA part took %.2f seconds to execute\n", time_taken);
        for (worker_rank = 1; worker_rank < num_procs; worker_rank++)
        {
            int dummy = 0;
            struct score_alignment alignment_scores_for_strings[chunk_size];
            MPI_Sendrecv(&dummy, 1, MPI_INT, worker_rank, PRINT, alignment_scores_for_strings, chunk_size, mpi_score_alignment_type, worker_rank, DONE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < chunk_size; i++)
            {
                printf("\nmy_rank [%d] for the string %s \nWe found that the max score alignment %d is from K  - %d and off set - %d  \n",
                       my_rank, alignment_scores_for_strings[i].str, alignment_scores_for_strings[i].score, alignment_scores_for_strings[i].K, alignment_scores_for_strings[i].off_set);
            }

            /* send STOP message. message has no data */
            MPI_Send(&dummy, 1, MPI_INT, worker_rank,
                     STOP, MPI_COMM_WORLD);
        }

        t_program = clock() - t_program;
        time_taken = ((double)t_program) / CLOCKS_PER_SEC; // in seconds
        fprintf(stderr, "\nProgram took %.2f seconds to execute\n", time_taken);
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
        struct score_alignment alignment_scores_for_strings[chunk_size];
        tag = status.MPI_TAG;
        do
        {
            if (tag == PRINT)
            {
                MPI_Send(alignment_scores_for_strings, chunk_size, mpi_score_alignment_type, ROOT, DONE, MPI_COMM_WORLD);
            }

            if (tag == GET)
            {
                for (int i = 0; i < chunk_size; i++)
                {
                    MPI_Recv(&size_str_to_check, 1, MPI_INT,
                             ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    MPI_Recv(alignment_scores_for_strings[i].str, (size_str_to_check + 1) * sizeof(char),
                             MPI_CHAR, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                tag = status.MPI_TAG;
            }
            if (tag == WORK)
            {
                for (int i = 0; i < chunk_size; i++)
                {
                    t_omp = clock();
                    if (how_to_caculate == THERE_IS_MATRIX_SCORE)
                        caculate_max_score_grade_table(alignment_scores_for_strings[i].str, first_str, matrix, &alignment_scores_for_strings[i]);
                    else
                        caculate_max_score_no_grade_table(alignment_scores_for_strings[i].str, first_str, &alignment_scores_for_strings[i]);
                    t_omp = clock() - t_omp;
                    time_taken = ((double)t_omp) / CLOCKS_PER_SEC; // in seconds
                    fprintf(stderr, "\nOpenMP part took %.2f seconds to execute\n", time_taken);
                }
            }
            MPI_Recv(&size_str_to_check, 1, MPI_INT,
                     ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            tag = status.MPI_TAG;

        } while (tag != STOP);
    }

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
