// eratosthenes.c
// Reseni IJC-DU1, priklad a), 20.3.2015
// Autor: Ondrej Vales, FIT
// Prelozeno: gcc 4.8.4

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "error.h"
#include "bit-array.h"

void Eratosthenes(BitArray_t pole)
{
    type n = BA_size(pole) - 1;
    for (type i = 2; i <= sqrt(n); i++)
    {
        if (BA_get_bit(pole, i) == 0)
        {
            for (type j = 2 * i; j <= n; j+=i)
            {
                BA_set_bit(pole,j,1);
            }
        }
    }
    return;
}
