#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
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
void Mutanat_Squence(char* str , int k)
{
	for (int i = 0; i <=(strlen(str)-k); i++)
	{
		if (toupper(*(str+i))== 'Z')
			*(str+i) = 'A';
		else
			*(str+i) = *(str+i)+1;
	}
	
}
int Sqn_Main_str(char** new_str, const char* first, int sqn, int length) {
    if (sqn + length >= strlen(first)) {
        return 0;  // Out-of-bounds
    }

    // char to_finish = first[sqn + length];

    // // Create a delimiter string for strtok
    // char delimiters[1] = {to_finish};

    // char* dummy = (char*)malloc((strlen(first) + 1) * sizeof(char));
    // if (!dummy) {
    //     return 0;
    // }

    // strcpy(dummy, first);
    
    // char* token = strtok(dummy, delimiters);
    // if (token) {
    //     *new_str = (char*)malloc((strlen(token) + 1) * sizeof(char));
    //     if (!*new_str) {
    //         free(dummy);
    //         return 0;
    //     }
    //     strcpy(*new_str, token);
    // }
    // printf("here in the func \n");
    // printf("%s ,%s\n",first, *new_str);
    // free(dummy);  // Free the allocated memory
    printf("\n\nlenght = %d",length);
    *new_str = (char*)malloc(length+1 * sizeof(char));
    if (!*new_str) {
            return 0;
        }
    for (int i = 0; i <=strlen(*new_str); i++)
    {
        *new_str[i] = first[i+sqn];
    }
    printf("here in the func \n");
    printf("%s ,%s\n",first, *new_str);
    return 1;
}
