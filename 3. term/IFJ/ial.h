/*
 * ial.h
 * Autor: Radek Vit
 * Autor: Tomas Zencak
 * Predmet: Formalni jazyky a prekladace
 * Projekt: Implementace interpretu jazyka IFJ15
 * Varianta zadani: a/2/II
 * Datum: 1. 10. 2015
 * Prelozeno: gcc 4.8
 * Tomas Zencak xzenca00
 * Ondrej Vales xvales03
 * Nikola Valesova xvales02
 * Radek Vit xvitra00
 */

#ifndef __IALH_
#define __IALH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "sem_anal_types.h"

#define HTAB_DEFAULT_SIZE 32

typedef SemVar HTdata;

typedef struct htitem HTItem;

struct htitem {
	char *key;
	HTdata value;
	HTItem *ptr;
};

typedef struct {
	size_t size;
	HTItem **table;
} HTab;

/**
 * @brief Makes the table ready to use.
 * @param table The table structure.
 * @return RET_EOK on success, RET_INTERN on failure
 */
int htab_init(HTab *table);

/**
 * @brief Makes the table ready to use.
 * @param table The table structure.
 * @param size The number of buckets in the table.
 * @return RET_EOK on success, RET_INTERN on failure
 */
int htab_init_sized(HTab *table, size_t size);

/**
 * @brief Inserts an item into the table. If there is an item with the same key, the new item will hide the old one.
 * @param table The table to insert into
 * @param key The key to insert
 * @param value The value to insert
 * @return RET_EOK on success, RET_INTERN on failure.
 */
int htab_insert(HTab *table, char* key, HTdata value);

/**
 * @brief Removes the first occurence of the given key from the table.
 * @param table The table to remove from.
 * @param key The key to remove.
 */
void htab_remove(HTab *table, char* key);

/**
 * @brief Searches the table for an item with the given key.
 * @param table The table to search in.
 * @param key The key to search for.
 * @return A pointer to the data corresponding to the given key, or NULL if the key wasn't found
 */
HTdata *htab_search(HTab *table, char *key);

/**
 * @brief Releases all the resources held by the table.
 * @param table The table to destroy.
 */
void htab_destroy(HTab *table);

typedef char Heap_element;

/**
 * @brief Sorts \p arr in ascending order using heapsort.
 * @param arr The array to sort
 * @param length the length of \p arr
 */
void heapsort(Heap_element *arr, int length);

/* returns starting position of first instance of search in s */
int find (char* s, char* search, int,int);

#endif
