// Soubor: htable_clear.c
// Reseni IJC-DU2, priklad 2
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#include "htable.h"

void htab_clear(htab_t *t)
{
    hashtab_listitem *aux;
    for (unsigned i = 0; i < t->htab_size; i++) // Pruchod seznamem.
    {
        hashtab_listitem *ptr = t->ptr[i];
        t->ptr[i] = NULL;
        while (ptr != NULL)		// Pruchod tabulkou.
        {
            aux = ptr;
            ptr = ptr->next;	// Posun na dalsi polozku.
            free(aux->key);		// Uvolneni pameti (klic i polozka sama).
            free(aux);
        }
    }
    return;
}
