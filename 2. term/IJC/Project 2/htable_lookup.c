// Soubor: htable_lookup.c
// Reseni IJC-DU2, priklad 2
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#include "htable.h"

struct htab_listitem *htab_lookup(htab_t *t, const char *key)
{
    unsigned int index = hash_function(key, t->htab_size);
    hashtab_listitem **ptr = (t->ptr) + index;

    while (*ptr != NULL && strcmp(key, (*ptr)->key) != 0)	// Vyhledani polozky v seznamu.
        ptr = &((*ptr)->next);

    if (*ptr == NULL)		// Polozku jsem nenalezl.
    {
        char *new_key = malloc((strlen(key) + 1) * sizeof(char));		// Alokace pameti pro klic
        if (new_key != NULL)
        {
            strcpy(new_key, key);
            hashtab_listitem *new_ptr = malloc(sizeof(hashtab_listitem));		// Alokace polozky seznamu.
            if (new_ptr != NULL)		// Naplneni nove polzky
            {
                new_ptr->key = new_key;
                new_ptr->data = 0;
                new_ptr->next = NULL;
                *ptr = new_ptr;
            }
            else
            {
                free(new_key);
            }
        }
    }

    return *ptr;
}
