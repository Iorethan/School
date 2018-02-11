/*
 * vector_ExprVar.h
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

#ifndef VECTOR_ExprVar_H_
#define VECTOR_ExprVar_H_

//This is essentially an emulation of the C++ std::vector.
//Only some functionality was implemented.

#include <stddef.h>
#include "sem_anal_types.h"

#define VECTOR_ExprVar_DEFAULT_SIZE 32

typedef struct{
	size_t size;
	size_t last;
	ExprVar *array;
}Vector_ExprVar;

int vector_ExprVar_init(Vector_ExprVar *vector);
int vector_ExprVar_init_sized(Vector_ExprVar *vector, int size);
void vector_ExprVar_destroy(Vector_ExprVar *vector);
//if error happens, the vector is still valid but the push didn't happen
int vector_ExprVar_push_back(Vector_ExprVar *vector, ExprVar value);
ExprVar vector_ExprVar_pop_back(Vector_ExprVar *vector);
ExprVar* vector_ExprVar_back(Vector_ExprVar *vector);
size_t vector_ExprVar_count(Vector_ExprVar *vector);
ExprVar* vector_ExprVar_at(Vector_ExprVar *vector, size_t index);
int vector_ExprVar_insert_at(Vector_ExprVar *vector, size_t index, ExprVar value);

#endif //STACK_ExprVar_H_
