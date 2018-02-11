/*
 * vector_FuncParam.h
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

#ifndef VECTOR_FuncParam_H_
#define VECTOR_FuncParam_H_

//This is essentially an emulation of the C++ std::vector.
//Only some functionality was implemented.

#include <stddef.h>
#include "sem_anal_types.h"

#define VECTOR_FuncParam_DEFAULT_SIZE 32

typedef struct{
	size_t size;
	size_t last;
	FuncParam *array;
}Vector_FuncParam;

int vector_FuncParam_init(Vector_FuncParam *vector);
int vector_FuncParam_init_sized(Vector_FuncParam *vector, int size);
void vector_FuncParam_destroy(Vector_FuncParam *vector);
//if error happens, the vector is still valid but the push didn't happen
int vector_FuncParam_push_back(Vector_FuncParam *vector, FuncParam value);
FuncParam vector_FuncParam_pop_back(Vector_FuncParam *vector);
FuncParam* vector_FuncParam_back(Vector_FuncParam *vector);
size_t vector_FuncParam_count(Vector_FuncParam *vector);
FuncParam* vector_FuncParam_at(Vector_FuncParam *vector, size_t index);
int vector_FuncParam_insert_at(Vector_FuncParam *vector, size_t index, FuncParam value);

#endif //STACK_FuncParam_H_
