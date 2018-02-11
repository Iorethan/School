/*
 * vector_UserFn.h
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

#ifndef VECTOR_UserFn_H_
#define VECTOR_UserFn_H_

//This is essentially an emulation of the C++ std::vector.
//Only some functionality was implemented.

#include <stddef.h>
#include "user_fn.h"

#define VECTOR_UserFn_DEFAULT_SIZE 32

typedef struct{
	size_t size;
	size_t last;
	UserFn *array;
}Vector_UserFn;

int vector_UserFn_init(Vector_UserFn *vector);
int vector_UserFn_init_sized(Vector_UserFn *vector, int size);
void vector_UserFn_destroy(Vector_UserFn *vector);
//if error happens, the vector is still valid but the push didn't happen
int vector_UserFn_push_back(Vector_UserFn *vector, UserFn value);
UserFn vector_UserFn_pop_back(Vector_UserFn *vector);
UserFn* vector_UserFn_back(Vector_UserFn *vector);
size_t vector_UserFn_count(Vector_UserFn *vector);
UserFn* vector_UserFn_at(Vector_UserFn *vector, size_t index);
int vector_UserFn_insert_at(Vector_UserFn *vector, size_t index, UserFn value);

#endif //STACK_UserFn_H_
