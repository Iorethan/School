// Soubor: htable_statistics.c
// Reseni IJC-DU2, priklad 2
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#include "htable.h"

void htab_statistics(htab_t *t)
{
    unsigned max = 0;
    unsigned min = UINT_MAX;
    unsigned sum = 0;
    for (unsigned i = 0; i < t->htab_size; i++) // Pruchod tabulkou.
    {
        hashtab_listitem *ptr = t->ptr[i];
        unsigned count = 0;
        while (ptr != NULL)		// Pruchod seznamem.
        {
            count++;			// V seznamu je dalsi polozka.
            ptr = ptr->next;
        }
        sum += count;			// Pocitani stat. udaju.
        if (count > max)
            max = count;
        if (count < min)
            min = count;
    }
    printf("avg: %.2f\nmin: %u\nmax: %u\n", ((double)sum) / t->htab_size, min, max);
    return;
}
