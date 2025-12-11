#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

typedef int (*GCF_func)(int, int);
typedef int *(*Sort_func)(int *, int);

void *lib_handle = NULL;
GCF_func gcf_func = NULL;
Sort_func sort_func = NULL;

int load_library(const char *libname)
{
    if (lib_handle)
        dlclose(lib_handle);

    lib_handle = dlopen(libname, RTLD_LAZY);
    if (!lib_handle)
    {
        printf("Ошибка загрузки: %s\n", dlerror());
        return 0;
    }

    gcf_func = (GCF_func)dlsym(lib_handle, "GCF");
    sort_func = (Sort_func)dlsym(lib_handle, "Sort");

    if (!gcf_func || !sort_func)
    {
        printf("Ошибка загрузки функций: %s\n", dlerror());
        dlclose(lib_handle);
        return 0;
    }

    printf("Загружена библиотека: %s\n", libname);
    return 1;
}

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
    if (!load_library("./libfuncs1.so"))
    {
        return 1;
    }

    printf("Программа 2: Динамическая загрузка библиотек\n");
    printf("Команды:\n");
    printf("  0 - переключить библиотеку (1/2)\n");
    printf("  1 A B - НОД чисел A и B\n");
    printf("  2 size n1 n2 ... - сортировка массива\n");
    printf("  q - выход\n\n");

    int current_lib = 1;
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
            current_lib = (current_lib == 1) ? 2 : 1;
            char libname[20];
            sprintf(libname, "./libfuncs%d.so", current_lib);

            if (load_library(libname))
            {
                printf("Переключено на библиотеку %d\n", current_lib);
            }
            else
            {
                current_lib = (current_lib == 1) ? 2 : 1;
            }
        }
        else if (line[0] == '1')
        {
            int a, b;
            if (sscanf(line + 1, "%d %d", &a, &b) == 2)
            {
                printf("НОД(%d, %d) = %d\n", a, b, gcf_func(a, b));
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

            int *sorted = sort_func(arr, size);
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

    if (lib_handle)
        dlclose(lib_handle);
    return 0;
}