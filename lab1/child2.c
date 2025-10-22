#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

int main()
{
    char buffer[BUFFER_SIZE];

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL)
    {

        for (int i = 0; buffer[i] != '\0' && buffer[i] != '\n'; i++)
        {
            if (isspace(buffer[i]))
            {
                buffer[i] = '_';
            }
        }

        printf("%s", buffer);
        fflush(stdout);
    }

    return 0;
}