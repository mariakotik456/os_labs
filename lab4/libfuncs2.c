#include <stdlib.h>

int GCF(int A, int B)
{
    A = abs(A);
    B = abs(B);

    int min = (A < B) ? A : B;
    for (int i = min; i >= 1; i--)
    {
        if (A % i == 0 && B % i == 0)
        {
            return i;
        }
    }
    return 1;
}

void quick_sort(int *arr, int low, int high)
{
    if (low < high)
    {
        int pivot = arr[high];
        int i = low - 1;

        for (int j = low; j < high; j++)
        {
            if (arr[j] < pivot)
            {
                i++;
                int temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }

        int temp = arr[i + 1];
        arr[i + 1] = arr[high];
        arr[high] = temp;

        int pi = i + 1;
        quick_sort(arr, low, pi - 1);
        quick_sort(arr, pi + 1, high);
    }
}

int *Sort(int *array, int size)
{
    if (!array || size <= 0)
        return NULL;

    int *result = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++)
        result[i] = array[i];

    quick_sort(result, 0, size - 1);
    return result;
}