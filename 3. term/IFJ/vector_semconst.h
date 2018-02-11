/*
 * vector_SemConst.h
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

#ifndef VECTOR_SemConst_H_
#define VECTOR_SemConst_H_

//This is essentially an emulation of the C++ std::vector.
//Only some functionality was implemented.

#include <stddef.h>
#include "sem_anal_types.h"

#define VECTOR_SemConst_DEFAULT_SIZE 32

typedef struct{
	size_t size;
	size_t last;
	SemConst *array;
}Vector_SemConst;

int vector_SemConst_init(Vector_SemConst *vector);
int vector_SemConst_init_sized(Vector_SemConst *vector, int size);
void vector_SemConst_destroy(Vector_SemConst *vector);
//if error happens, the vector is still valid but the push didn't happen
int vector_SemConst_push_back(Vector_SemConst *vector, SemConst value);
SemConst vector_SemConst_pop_back(Vector_SemConst *vector);
SemConst* vector_SemConst_back(Vector_SemConst *vector);
size_t vector_SemConst_count(Vector_SemConst *vector);
SemConst* vector_SemConst_at(Vector_SemConst *vector, size_t index);
int vector_SemConst_insert_at(Vector_SemConst *vector, size_t index, SemConst value);

#endif //STACK_SemConst_H_
