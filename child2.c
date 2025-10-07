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
//         // Заменяем пробелы на подчеркивания (игнорируя символ новой строки)
//         int len = strlen(buffer);
//         for (int i = 0; i < len && buffer[i] != '\n'; i++)
//         {
//             if (isspace(buffer[i]))
//             {
//                 buffer[i] = '_';
//             }
//         }

//         // Выводим результат (будет перенаправлен в Parent)
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
        // Заменяем пробелы на подчеркивания (игнорируя символ новой строки)
        int len = strlen(buffer);
        for (int i = 0; i < len && buffer[i] != '\n'; i++)
        {
            if (isspace(buffer[i]))
            {
                buffer[i] = '_';
            }
        }

        // Выводим результат
        printf("%s", buffer);
        fflush(stdout);
    }

    // fgets вернул NULL - значит получен EOF, выходим
    return 0;
}