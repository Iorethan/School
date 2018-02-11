// Soubor: htable.h
// Reseni IJC-DU2, priklad 2
// Datum: 24. 4. 2015
// Autor: Ondrej Vales, xvales03
// Fakulta: FIT VUTBR
// Prelozeno: gcc 4.8

#ifndef HTABLE_H_INCLUDED
#define HTABLE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct htab_listitem{
    char *key;
    unsigned int data;
    struct htab_listitem *next;
} hashtab_listitem;

typedef struct {
    unsigned int htab_size;
    struct htab_listitem *ptr[];
} htab_t;

htab_t *htab_init(unsigned size);
struct htab_listitem *htab_lookup(htab_t *t, const char *key);
void htab_foreach(htab_t *t, void (*function)(const char* key, unsigned int value));
void htab_remove(htab_t *t, const char *key);
void htab_clear(htab_t *t);
void htab_free(htab_t *t);
void htab_statistics(htab_t *t);
unsigned int hash_function(const char *str, unsigned htab_size);

#endif // HTABLE_H_INCLUDED
