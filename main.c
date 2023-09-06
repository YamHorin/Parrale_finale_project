#include <omp.h>
#include <stdlib.h>
#include <ctype.h>
#include "mpi.h"
#include "cFunctions.h"
#include <cstring>

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
   char* str;
   int sqn;
   int MS;
   int score;
};


#define MATRIX_SIZE 26
#define ROOT 0

int lenght_first_str;
int number_strings;
char* first_str;
int matrix [MATRIX_SIZE][MATRIX_SIZE];

int readMatrixFromFile(const char* filename, int matrix[MATRIX_SIZE][MATRIX_SIZE]);
void init(int argc, char **argv); 
extern int computeOnGPU(const char  *s1, const char *s2);
extern int computeOnGPUWithMatrix(const char  *s1, const char *s2 ,const int matrix[MATRIX_SIZE][MATRIX_SIZE]);

int main(int argc, char *argv[]) 
{

    int my_rank, num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    //making new type-struct score_alignment
    MPI_Datatype mpi_score_alignment_type;
    MPI_Datatype types[4] = {MPI_CHAR, MPI_INT, MPI_INT, MPI_INT};
    int block_lengths[4] = {0}; // Initialized to zeros
    MPI_Aint displacements[4];
    struct score_alignment temp; // Used to calculate displacements

    // Calculate displacements
    MPI_Get_address(&temp.str, &displacements[0]);
    MPI_Get_address(&temp.sqn, &displacements[1]);
    MPI_Get_address(&temp.MS, &displacements[2]);
    MPI_Get_address(&temp.score, &displacements[3]);

    for (int i = 3; i > 0; i--) {
        displacements[i] -= displacements[0];
    }
    displacements[0] = 0;

    // Create the custom data type
    MPI_Type_create_struct(4, block_lengths, displacements, types, &mpi_score_alignment_type);
    MPI_Type_commit(&mpi_score_alignment_type);

    if (my_rank==0)
    {
      init(argc, argv);

      int int_enum = (int)how_to_caculate;
      MPI_Bcast(&int_enum , 1 , MPI_INT , ROOT , MPI_COMM_WORLD);
      if (how_to_caculate==THERE_IS_MATRIX_SCORE)
           MPI_Bcast(matrix , MATRIX_SIZE*MATRIX_SIZE , MPI_INT , ROOT , MPI_COMM_WORLD);
      MPI_Status status;
      double t_start = MPI_Wtime();
      MPI_Bcast(&lenght_first_str , 1 , MPI_INT , ROOT , MPI_COMM_WORLD);
      MPI_Bcast(first_str , strlen(first_str)*sizeof(char) , MPI_CHAR , ROOT , MPI_COMM_WORLD);
      char* str_to_send;
      int str_lenght;
      for (int worker_rank = 1; worker_rank < num_procs; worker_rank++)
        {
            str_to_send = createDynStr();
            str_lenght = strlen(str_to_send)+1;
            #ifdef DEBUG
                printf("send to rank %d -%s\n",worker_rank , str_to_send);

            #endif
            MPI_Send(&str_lenght , 1 , MPI_INT , worker_rank  , WORK , MPI_COMM_WORLD);
            MPI_Send(str_to_send, str_lenght*sizeof(char) , MPI_CHAR , worker_rank, WORK, MPI_COMM_WORLD);
        }
        int str_send = num_procs-1; 
        int tasks = number_strings/(num_procs-1);
        if (number_strings%(num_procs-1))
            tasks++;
        for (int tasks_done = 0; tasks_done<number_strings; tasks_done++)
        {
             struct  score_alignment localMax;
            MPI_Recv(&localMax, 1, mpi_score_alignment_type, MPI_ANY_SOURCE,
		                    DONE, MPI_COMM_WORLD, &status);
            printf("\nfor the string %s \n, we found that the max score alignment %d is from MS %d and %d sqn \n",
            localMax.str , localMax.score , localMax.MS , localMax.sqn);
            int tasks_not_sent_yet = tasks - str_send;
            if (tasks_not_sent_yet > 0) {
                    str_to_send = createDynStr();
                    str_lenght = strlen(str_to_send);
                    MPI_Send(&str_lenght , 1 , MPI_CHAR , status.MPI_SOURCE  , WORK , MPI_COMM_WORLD);
                    MPI_Send(str_to_send, strlen(str_to_send)*sizeof(char) , MPI_CHAR , status.MPI_SOURCE, WORK, MPI_COMM_WORLD);
                    str_send++;
                }
            else {
                    /* send STOP message. message has no data */
                    int dummy;
                    MPI_Send(&dummy,1, MPI_INT, status.MPI_SOURCE,
                            STOP, MPI_COMM_WORLD);
                }
        
        }
        fprintf(stderr,"sequential time: %f secs\n", MPI_Wtime() - t_start);

    }
    else
    {
        int size_str_to_check ,enumGet;
        char* str_to_check;
        MPI_Bcast(&enumGet , 1 , MPI_INT , ROOT , MPI_COMM_WORLD);
        how_to_caculate = (enum matrix_score) enumGet;
        if (how_to_caculate==THERE_IS_MATRIX_SCORE)
           MPI_Bcast(matrix  , MATRIX_SIZE*MATRIX_SIZE , MPI_INT , ROOT , MPI_COMM_WORLD);
        MPI_Bcast(&lenght_first_str , 1 , MPI_INT , ROOT , MPI_COMM_WORLD);
        first_str = (char*)malloc(lenght_first_str*sizeof(char));
        MPI_Bcast(first_str , lenght_first_str*sizeof(char) , MPI_CHAR , ROOT , MPI_COMM_WORLD);
        MPI_Status status;
        int tag,sqn_taries;
        do
        {

            MPI_Recv(&size_str_to_check , 1 , MPI_INT , 
            ROOT,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
            tag = status.MPI_TAG;
            struct score_alignment temp_Max;
            if (tag==WORK)
            {
                str_to_check = (char*)malloc(size_str_to_check*sizeof(char));
                if (!str_to_check)
                {
                    perror("malloc");
                    exit(1);
                }
                MPI_Recv(str_to_check, size_str_to_check*sizeof(char), 
                MPI_CHAR , ROOT,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                #ifdef DEBUG
                    printf("rank = %d tag = %d\n",my_rank,status.MPI_TAG);
                    printf("got str:%s \n",str_to_check);

                #endif

                #pragma omp declare reduction(AS_min_func : struct score_alignment : \
                        omp_out = (omp_out.score > omp_in.score ? omp_out : omp_in)) \
                        initializer(omp_priv = omp_orig)
                
                temp_Max.score = -1;
                temp_Max.str  = (char*)malloc(size_str_to_check*sizeof(char));
                if (!temp_Max.str)
                {
                    perror("malloc");
                    exit(1);
                }
                
                sqn_taries = (size_str_to_check<lenght_first_str)? (lenght_first_str-size_str_to_check)
                : (size_str_to_check-lenght_first_str);
                char temp_first_str [size_str_to_check] ;
                #pragma omp parallel for reduction(AS_min_func : temp_Max)
                for (int i = 0; i < sqn_taries; i++)
                {
                    temp_Max.sqn = i;
                    for (int j = 0; j <=size_str_to_check; j++)
                    {
                        temp_first_str[i] = *(first_str+j+temp_Max.sqn);
                    }
                    #ifdef DEBUG
                        printf(" %s str %s , sqn_number = %d \n" ,temp_first_str, str_to_check  , i);
                    #endif
                    #pragma omp parallel for reduction(AS_min_func : temp_Max)
                    for (int i = 0; i < size_str_to_check; i++)
                    {
                        temp_Max.MS = i;
                    #ifdef DEBUG
                        printf("str %s , <MS> = %d \n" , str_to_check  , i);
                    #endif
                        strcpy(temp_Max.str , str_to_check);   
                        Mutanat_Squence(temp_Max.str , i);
                        //caculate result
                        if (how_to_caculate==NO_MATRIX_SCORE)
                            temp.score = computeOnGPU(temp_first_str , temp_Max.str);
                        else
                            temp.score = computeOnGPUWithMatrix(temp_first_str , temp_Max.str , matrix);
                    }    
                }
                
            }
            strcpy(temp_Max.str , str_to_check);
            MPI_Send(&temp_Max , 1  , mpi_score_alignment_type , ROOT , DONE , MPI_COMM_WORLD);
            free(str_to_check);
            free(temp_Max.str); 
        } while (tag != STOP);
       
    }
    MPI_Barrier(MPI_COMM_WORLD);
    free(first_str);
    MPI_Type_free(&mpi_score_alignment_type);
    MPI_Finalize();
    return 0;

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


int readMatrixFromFile(const char* filename, int matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1; // Indicate failure
    }

    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            if (fscanf(file, "%d", &matrix[i][j]) != 1) {  // assuming integers in the matrix
                perror("Error reading matrix values");
                fclose(file);
                return -2;  // Indicate reading error
            }
        }
    }

    fclose(file);
    return 0;  // Success
}
