#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libfuncs.h"

void print_array(int *arr, int size)
{
    printf("[");
    for (int i = 0; i < size; i++)
    {
        printf("%d", arr[i]);
        if (i < size - 1)
        {
            printf(", ");
        }
    }
    printf("]\n");
}

int main()
{
    printf("Программа 1: Использование библиотеки при линковке\n");
    printf("Команды:\n");
    printf("  0 - справка\n");
    printf("  1 A B - НОД чисел A и B\n");
    printf("  2 size n1 n2 ... - сортировка массива\n");
    printf("  q - выход\n\n");

    char line[256];
    while (1)
    {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin))
            break;

        line[strcspn(line, "\n")] = 0;

        if (line[0] == 'q')
            break;

        if (line[0] == '0')
        {
            printf("Справка:\n");
            printf("  1 A B - вычисление НОД чисел A и B\n");
            printf("  2 size n1 n2 ... - сортировка массива\n");
            printf("      size - размер массива\n");
            printf("      n1 n2 ... - элементы массива\n");
        }
        else if (line[0] == '1')
        {
            int a, b;
            if (sscanf(line + 1, "%d %d", &a, &b) == 2)
            {
                printf("НОД(%d, %d) = %d\n", a, b, GCF(a, b));
            }
            else
            {
                printf("Ошибка формата. Используйте: 1 A B\n");
            }
        }
        else if (line[0] == '2')
        {
            int size;
            char *ptr = line + 2;

            if (sscanf(ptr, "%d", &size) != 1 || size <= 0)
            {
                printf("Ошибка: укажите корректный размер массива (> 0)\n");
                continue;
            }

            while (*ptr && *ptr != ' ')
                ptr++;
            while (*ptr == ' ')
                ptr++;

            int *arr = malloc(size * sizeof(int));
            if (arr == NULL)
            {
                printf("Ошибка: не удалось выделить память\n");
                continue;
            }

            int i;
            for (i = 0; i < size; i++)
            {
                if (*ptr == '\0')
                {
                    break;
                }
                if (sscanf(ptr, "%d", &arr[i]) != 1)
                {
                    break;
                }
                while (*ptr && *ptr != ' ')
                    ptr++;
                while (*ptr == ' ')
                    ptr++;
            }

            if (i != size)
            {
                printf("Ошибка: указано %d элементов, но требуется %d\n", i, size);
                free(arr);
                continue;
            }

            printf("Исходный массив: ");
            print_array(arr, size);

            int *sorted = Sort(arr, size);
            if (sorted)
            {
                printf("Отсортированный массив: ");
                print_array(sorted, size);
                free(sorted);
            }
            else
            {
                printf("Ошибка при сортировке\n");
            }

            free(arr);
        }
        else
        {
            if (line[0] != '\0')
            {
                printf("Неизвестная команда. Доступные команды: 0, 1, 2, q\n");
            }
        }
    }

    return 0;
}