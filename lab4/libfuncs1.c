#include <stdlib.h>

int GCF(int A, int B)
{
    A = abs(A);
    B = abs(B);

    while (A != 0 && B != 0)
    {
        if (A > B)
        {
            A = A % B;
        }
        else
        {
            B = B % A;
        }
    }
    return A + B;
}

int *Sort(int *array, int size)
{
    if (!array || size <= 0)
        return NULL;

    int *result = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++)
        result[i] = array[i];

    for (int i = 0; i < size - 1; i++)
    {
        for (int j = 0; j < size - i - 1; j++)
        {
            if (result[j] > result[j + 1])
            {
                int temp = result[j];
                result[j] = result[j + 1];
                result[j + 1] = temp;
            }
        }
    }
    return result;
}