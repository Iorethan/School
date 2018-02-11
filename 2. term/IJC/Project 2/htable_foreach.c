// Soubor: htable_foreach.c
// Reseni IJC-DU2, priklad 2
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#include "htable.h"

void htab_foreach(htab_t *t, void (*function)(const char* key, unsigned int value))
{
    for (unsigned i = 0; i < t->htab_size; i++)	// Pruchod tabulkou.
    {
        hashtab_listitem *ptr = t->ptr[i];
        while (ptr != NULL)		// Pruchod seznamem.
        {
            function(ptr->key, ptr->data);	// Volani funkce pro kazdou polozku
            ptr = ptr->next;
        }
    }
    return;
}
