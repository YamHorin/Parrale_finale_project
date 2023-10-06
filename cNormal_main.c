#include <omp.h>
#include <stdlib.h>
#include <ctype.h>
#include "mpi.h"
#include "cFunctions.h"
#include <cstring>


#define MATRIX_SIZE 26
#define ROOT 0
#define MAX_STRING_SIZE 3000
#define VERY_LONG 500
//enums


enum matrix_score
{
    THERE_IS_MATRIX_SCORE ,
    NO_MATRIX_SCORE
}how_to_caculate;

enum tags
{
    WORK,
    STOP,
    DONE
};
struct score_alignment
{
   char str [MAX_STRING_SIZE];
   int off_set;
   int K;
   int score;
};

//static values

int lenght_first_str;
int number_strings;
char* first_str;
int matrix [MATRIX_SIZE][MATRIX_SIZE];

void init(int argc, char **argv );
int caculate_result_without_matrix(const char *s2 , int off_set);
int calculate_result_with_matrix(const char* s2, int matrix[MATRIX_SIZE][MATRIX_SIZE] , int off_set);


int main(int argc, char *argv[]) 
{

    int my_rank, num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (num_procs!=1)
    {
        printf("can only work with one process \n");
        MPI_Finalize();
        return 1;
    }
    printf("this is a normal run\n");
    double t_start = MPI_Wtime();
      init(argc, argv);
      char* str_to_check;
      int score;
        for (int tasks_done = 0; tasks_done<number_strings; tasks_done++)
        {
            str_to_check = createDynStr();
            struct  score_alignment localMax;
            localMax.score =0;
            localMax.K =0;
            localMax.off_set = 0;
            int size_str_to_check = strlen(str_to_check);
            int lenght_first_str = strlen(first_str);
            int sqn_taries = (size_str_to_check<lenght_first_str)? (lenght_first_str-size_str_to_check): (size_str_to_check-lenght_first_str);
            strncpy(localMax.str , str_to_check ,MAX_STRING_SIZE-1);
            localMax.str[MAX_STRING_SIZE] = '\0';   
            for (int off_set = 0; off_set <= sqn_taries; off_set++)
                {
                    
                    for (int k =0; k < size_str_to_check; k++)
                    { 
                            strcpy(str_to_check ,localMax.str);
                          Mutanat_Squence(str_to_check ,k,size_str_to_check);
                            if (how_to_caculate==NO_MATRIX_SCORE)
                                 score = caculate_result_without_matrix(str_to_check , off_set);
                            else
                                 score = calculate_result_with_matrix(str_to_check , matrix,off_set);

                            if (localMax.score<score)
                            {
                                localMax.K = k;
                                localMax.off_set = off_set;
                                localMax.score = score;
                            }

                    }
                }

            printf("\nmy_rank [0] for the string %s \nI found that the max score alignment %d is from K  - %d and off set - %d  \n",
            localMax.str, localMax.score , localMax.K , localMax.off_set);
        }
    fprintf(stderr,"sequential time: %f secs\n", MPI_Wtime() - t_start);
     MPI_Finalize();
    return EXIT_SUCCESS;
    }



void init(int argc, char **argv )
{
    first_str = createDynStr();
    if (first_str==NULL)
    {
      fprintf(stderr,"line 1 should be the first string\n");
        exit(1);
    }
    lenght_first_str = strlen(first_str);
    if (!(scanf("%d",&number_strings)))
    {
            fprintf(stderr,"line 2 should be the numebr of strings to check\n");
            exit(1);
    }
    how_to_caculate = NO_MATRIX_SCORE;
    if (argc==2)
    {
        how_to_caculate = THERE_IS_MATRIX_SCORE;
        if (readMatrixFromFile(argv[1], matrix) == 0) 
            printf("Matrix read successfully!\n");
        else 
            printf("There was an error reading the matrix.\n");
    

    }
    
}

int caculate_result_without_matrix(const char *s2 , int off_set)
{
    int length= strlen(s2);
    int result  = 0;
    for (int i = 0; i < length; i++)
    {
        if (*((first_str+i)+off_set)==s2[i])
            result++;
    }
    return result;

}

int calculate_result_with_matrix(const char* s2, int matrix[MATRIX_SIZE][MATRIX_SIZE] , int off_set) {
    int length = strlen(s2);
    int result = 0;

    for (int i = 0; i < length; i++) {
        int x = *(first_str+i+off_set) - 'A';
        int y = s2[i] - 'A';

        if (x < 0 || x >= MATRIX_SIZE || y < 0 || y >= MATRIX_SIZE) {
            // Handle out-of-bounds characters.
            return -1; // or any appropriate error code
        }
        result += matrix[x][y];
    }

    return result;
}