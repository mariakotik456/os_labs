// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <ctype.h>

// #define BUFFER_SIZE 1024

// int main()
// {
//     char buffer[BUFFER_SIZE];

//     // Полностью отключаем буферизацию
//     setvbuf(stdin, NULL, _IONBF, 0);
//     setvbuf(stdout, NULL, _IONBF, 0);

//     while (fgets(buffer, BUFFER_SIZE, stdin) != NULL)
//     {
//         // Конвертируем в верхний регистр (игнорируя символ новой строки)
//         int len = strlen(buffer);
//         for (int i = 0; i < len && buffer[i] != '\n'; i++)
//         {
//             buffer[i] = toupper(buffer[i]);
//         }

//         // Выводим результат (будет перенаправлен в Child2)
//         printf("%s", buffer);
//         fflush(stdout);
//     }

//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

int main()
{
    char buffer[BUFFER_SIZE];

    // Отключаем буферизацию
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    // Читаем пока не получим EOF
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL)
    {
        // Конвертируем в верхний регистр (игнорируя символ новой строки)
        int len = strlen(buffer);
        for (int i = 0; i < len && buffer[i] != '\n'; i++)
        {
            buffer[i] = toupper(buffer[i]);
        }

        // Выводим результат
        printf("%s", buffer);
        fflush(stdout);
    }

    // fgets вернул NULL - значит получен EOF, выходим
    return 0;
}