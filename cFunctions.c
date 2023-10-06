#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>


#define MATRIX_SIZE 26
#define ROOT 0
#define MAX_STRING_SIZE 3000
#define VERY_LONG 500

char *createDynStr()
{
	char *str;
	char temp[MAX_STRING_SIZE];
	scanf("%s", temp);
	str = (char *)malloc((strlen(temp)) * sizeof(char));
	if (!str)
		return NULL;
	strcpy(str, temp);
	return str;
}

void Mutanat_Squence(char *str, int k, int size_str)
{
	for (int i = 0; i < size_str; i++)
	{
		if (i>=k)
		{
			if (str[i] =='z')
				str[i] = 'a';
			else if (str[i] =='Z')
				str[i] = 'A';
			else
				str[i]++;
		}
		str[i] = toupper(str[i]);
	}
}

int readMatrixFromFile(const char *filename, int matrix[MATRIX_SIZE][MATRIX_SIZE])
{
	FILE *file = fopen(filename, "r");
	if (file == NULL)
	{
		perror("Error opening file");
		return -1;
	}

	for (int i = 0; i < MATRIX_SIZE; i++)
	{
		for (int j = 0; j < MATRIX_SIZE; j++)
		{
			if (fscanf(file, "%d", &matrix[i][j]) != 1)
			{
				perror("Error reading matrix values");
				fclose(file);
				return -2;
			}
		}
	}

	fclose(file);
	return 0;
}
