// Soubor: htable_free.c
// Reseni IJC-DU2, priklad 2
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#include "htable.h"

void htab_free(htab_t *t)
{
    htab_clear(t);
    free(t);
    return;
}
