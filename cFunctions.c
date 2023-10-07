#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>

#define MATRIX_SIZE 26
#define ROOT 0
#define MAX_STRING_SIZE 3000
#define VERY_LONG 500

// Function to create a dynamically allocated string from user input
char *createDynStr()
{
    char *str;
    char temp[MAX_STRING_SIZE];
    scanf("%s", temp); // Read a string from the user
    str = (char *)malloc((strlen(temp)) * sizeof(char)); // Allocate memory for the string
    if (!str)
        return NULL; // Return NULL if memory allocation fails
    strcpy(str, temp); // Copy the input string to the dynamically allocated memory
    return str;
}

// Function to mutate a given string 'str' starting from index 'k' and make it uppercase
void Mutanat_Squence(char *str, int k, int size_str)
{
    for (int i = 0; i < size_str; i++)
    {
        if (i >= k)
        {
            if (str[i] == 'z')
                str[i] = 'a'; // Wrap 'z' to 'a'
            else if (str[i] == 'Z')
                str[i] = 'A'; // Wrap 'Z' to 'A'
            else
                str[i]++; // Increment the character
        }
        str[i] = toupper(str[i]); // Convert the character to uppercase
    }
}

// Function to read a matrix from a file and store it in a 2D array 'matrix'
int readMatrixFromFile(const char *filename, int matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    FILE *file = fopen(filename, "r"); // Open the file for reading
    if (file == NULL)
    {
        perror("Error opening file"); // Print an error message if opening the file fails
        return -1;
    }

    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            if (fscanf(file, "%d", &matrix[i][j]) != 1)
            {
                perror("Error reading matrix values"); // Print an error message if reading the matrix values fails
                fclose(file); // Close the file
                return -2;
            }
        }
    }

    fclose(file); // Close the file
    return 0; // Return 0 if the operation is successful
}