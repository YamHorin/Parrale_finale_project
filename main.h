#define MATRIX_SIZE 26
#define ROOT 0
#define MAX_STRING_SIZE 3000


enum tags
{
    WORK,
    STOP,
    DONE,
    PRINT,
    GET
};

enum matrix_score 
{
    THERE_IS_MATRIX_SCORE,
    NO_MATRIX_SCORE
};


enum matrix_score how_to_caculate;


void init(int argc, char **argv);
