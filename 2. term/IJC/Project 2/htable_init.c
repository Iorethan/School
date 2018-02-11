// Soubor: htable_init.c
// Reseni IJC-DU2, priklad 2
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#include "htable.h"

htab_t *htab_init(unsigned size)
{
    htab_t *tab;
    tab = malloc(sizeof(htab_t) + size * sizeof(hashtab_listitem *));	// Alokace hlavicky a prislusneho seznamu.
    if (tab != NULL)		// Nastaveni obsahu tabulky.
    {
        tab->htab_size = size;
        for (unsigned i = 0; i < size; i++)
            tab->ptr[i] = NULL;
    }
    return tab;
}
