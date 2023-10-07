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

// static values
int chunk_size;
int lenght_first_str;
int number_strings;
char *first_str;
int matrix[MATRIX_SIZE][MATRIX_SIZE];

int main(int argc, char *argv[])
{
    clock_t t_program;
    int my_rank, num_procs;
    double time_taken;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Datatype mpi_score_alignment_type;    // datatype to sending the result to proc 0 for the other procs
    make_datatype(&mpi_score_alignment_type); // func in omp_mpi_functions
    if (my_rank == 0)
    {
        printf("this is a paralell run\n");
        init(argc, argv); // reads the first str and file grade table
        int int_enum = (int)how_to_caculate;

        MPI_Bcast(&int_enum, 1, MPI_INT, ROOT, MPI_COMM_WORLD); // enum that tells if we have grade table

        if (how_to_caculate == THERE_IS_MATRIX_SCORE)
            MPI_Bcast(matrix, MATRIX_SIZE * MATRIX_SIZE, MPI_INT, ROOT, MPI_COMM_WORLD); // bcast the grade table

        MPI_Bcast(&lenght_first_str, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
        MPI_Bcast(first_str, lenght_first_str * sizeof(char), MPI_CHAR, ROOT, MPI_COMM_WORLD); // bcast the first str of the data

        int worker_rank;
        int master_chunk_size, chunk_size;
        master_chunk_size = number_strings >= num_procs ? number_strings / num_procs : num_procs / number_strings;
        /*

        master_chunk size - chunk size for procs 0 that add an number_strings % num_procs
        chunk size  = chunk size that every procs gets
        */
        if ((number_strings % num_procs) != 0)
            master_chunk_size += number_strings >= num_procs ? number_strings % num_procs : num_procs % number_strings;
        char strings_to_check[master_chunk_size][MAX_STRING_SIZE];
        for (int i = 0; i < master_chunk_size; i++)
        {
            scanf("%s", strings_to_check[i]);
        }
        chunk_size = (number_strings - master_chunk_size) >= (num_procs - 1) ? (number_strings - master_chunk_size) / (num_procs - 1) : (num_procs - 1) / (number_strings - master_chunk_size);
        struct score_alignment strings_to_give[chunk_size];
        // reading and sending strings to check to each procs
        for (worker_rank = 1; worker_rank < num_procs; worker_rank++)
        {
            for (int i = 0; i < chunk_size; i++)
            {
                scanf("%s", strings_to_give[i].str);
            }
            MPI_Send(&chunk_size, 1, MPI_INT, worker_rank, GET, MPI_COMM_WORLD);
            MPI_Send(strings_to_give, chunk_size, mpi_score_alignment_type, worker_rank, WORK, MPI_COMM_WORLD);
        }
        t_program = clock();
        // for each string in strings_to_check we caculate the result and print-the print happend in the caculate cuda
        for (int i = 0; i < master_chunk_size; i++)
        {
            char *str_to_check = strings_to_check[i];
            int score;

            if (how_to_caculate == NO_MATRIX_SCORE)
                score = caculate_cuda_without_matrix(str_to_check, first_str, my_rank);
            else
                score = caculate_cuda(str_to_check, first_str, matrix, my_rank);
        }

        struct score_alignment alignment_scores_finale[chunk_size];
        // for each procs that isn't the master - we recv the info and print it
        for (int worker_rank = 1; worker_rank < num_procs; worker_rank++)
        {
            MPI_Recv(alignment_scores_finale, chunk_size, mpi_score_alignment_type, worker_rank, DONE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < chunk_size; i++)
            {
                printf("We found that the max score alignment %d is from K - %d and off set - %d\n", alignment_scores_finale[i].score, alignment_scores_finale[i].K, alignment_scores_finale[i].off_set);
            }
        }
        // the main time that took for the program
        t_program = clock() - t_program;
        time_taken = ((double)t_program) / CLOCKS_PER_SEC; // in seconds
        fprintf(stderr, "\nparallel program took %.2f seconds to execute\n", time_taken);
    }

    else
    {
        int enumGet;
        MPI_Bcast(&enumGet, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
        how_to_caculate = (enum matrix_score)enumGet; // casting the int into enum
        if (how_to_caculate == THERE_IS_MATRIX_SCORE)
            MPI_Bcast(matrix, MATRIX_SIZE * MATRIX_SIZE, MPI_INT, ROOT, MPI_COMM_WORLD); // bcast the grade table

        MPI_Bcast(&lenght_first_str, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
        first_str = (char *)malloc(lenght_first_str * sizeof(char));
        MPI_Bcast(first_str, lenght_first_str * sizeof(char), MPI_CHAR, ROOT, MPI_COMM_WORLD); // bcast the first str in the data
        int chunk_size;

        MPI_Recv(&chunk_size, 1, MPI_INT,
                 ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        struct score_alignment alignment_scores_for_strings[chunk_size]; // gets the strings to check from the master
        char strings_to_check[chunk_size][MAX_STRING_SIZE];
        MPI_Recv(alignment_scores_for_strings, chunk_size, mpi_score_alignment_type,
                 ROOT, WORK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

       
        // using a parallel space in the main
        #pragma omp parallel for shared(alignment_scores_for_strings)
        for (int i = 0; i < chunk_size; i++)
        {

            if (how_to_caculate == THERE_IS_MATRIX_SCORE)
                caculate_max_score_grade_table(alignment_scores_for_strings[i].str, first_str, matrix, &alignment_scores_for_strings[i]);
            else
                caculate_max_score_no_grade_table(alignment_scores_for_strings[i].str, first_str, &alignment_scores_for_strings[i]);
        }
        
        MPI_Send(alignment_scores_for_strings, chunk_size, mpi_score_alignment_type, ROOT, DONE, MPI_COMM_WORLD);
    }

    free(first_str);                          // free chr*
    MPI_Type_free(&mpi_score_alignment_type); // free space type
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