/*
 * vector_Rule.h
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

#ifndef VECTOR_Rule_H_
#define VECTOR_Rule_H_

//This is essentially an emulation of the C++ std::vector.
//Only some functionality was implemented.

#include <stddef.h>
#include "synt_anal_types.h"

#define VECTOR_Rule_DEFAULT_SIZE 32

typedef struct{
	size_t size;
	size_t last;
	Rule *array;
}Vector_Rule;

int vector_Rule_init(Vector_Rule *vector);
int vector_Rule_init_sized(Vector_Rule *vector, int size);
void vector_Rule_destroy(Vector_Rule *vector);
//if error happens, the vector is still valid but the push didn't happen
int vector_Rule_push_back(Vector_Rule *vector, Rule value);
Rule vector_Rule_pop_back(Vector_Rule *vector);
Rule* vector_Rule_back(Vector_Rule *vector);
size_t vector_Rule_count(Vector_Rule *vector);
Rule* vector_Rule_at(Vector_Rule *vector, size_t index);
int vector_Rule_insert_at(Vector_Rule *vector, size_t index, Rule value);

#endif //STACK_Rule_H_
