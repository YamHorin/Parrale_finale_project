#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
#define MATRIX_SIZE 26
#define ROOT 0
#define MAX_STRING_SIZE 3000
#define VERY_LONG 500
char* createDynStr()
{
	char* str;
	char temp[1000];
	scanf("%s",temp);
	str = (char*)malloc((strlen(temp))*sizeof(char));
	if (!str)
		return NULL;
	strcpy(str, temp);
	return str;
}

void Mutanat_Squence(char* str , int k , int size_str)
{
	for (int i = k; i <=size_str; i++)
	{
		if (toupper(*(str+i))>= 'Z')
			*(str+i) = 'A';
		if (i==size_str)
			*(str+i) = '\0';
		else
			*(str+i) = toupper(*((str+i))+1);
		

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





