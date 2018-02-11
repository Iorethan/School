// Soubor: htable_remove.c
// Reseni IJC-DU2, priklad 2
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#include "htable.h"

void htab_remove(htab_t *t, const char *key)
{
    unsigned int index = hash_function(key, t->htab_size);
    hashtab_listitem **ptr = (t->ptr) + index;		// Ziskam ukazaten na prislusnou polzku v tabulce

    while (*ptr != NULL && strcmp(key, (*ptr)->key) != 0)	// Vyhledani dane polozky v seznamu
        ptr = &((*ptr)->next);

    if (*ptr != NULL)		// Pokud sem polzku nasel odstranim ji.
    {
        hashtab_listitem *aux = *ptr;
        *ptr = aux->next;
        free(aux->key);
        free(aux);
    }

    return;
}
