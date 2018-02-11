/*
 * vector_SemInstr.h
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

#ifndef VECTOR_SemInstr_H_
#define VECTOR_SemInstr_H_

//This is essentially an emulation of the C++ std::vector.
//Only some functionality was implemented.

#include <stddef.h>
#include "sem_anal_types.h"

#define VECTOR_SemInstr_DEFAULT_SIZE 32

typedef struct{
	size_t size;
	size_t last;
	SemInstr *array;
}Vector_SemInstr;

int vector_SemInstr_init(Vector_SemInstr *vector);
int vector_SemInstr_init_sized(Vector_SemInstr *vector, int size);
void vector_SemInstr_destroy(Vector_SemInstr *vector);
//if error happens, the vector is still valid but the push didn't happen
int vector_SemInstr_push_back(Vector_SemInstr *vector, SemInstr value);
SemInstr vector_SemInstr_pop_back(Vector_SemInstr *vector);
SemInstr* vector_SemInstr_back(Vector_SemInstr *vector);
size_t vector_SemInstr_count(Vector_SemInstr *vector);
SemInstr* vector_SemInstr_at(Vector_SemInstr *vector, size_t index);
int vector_SemInstr_insert_at(Vector_SemInstr *vector, size_t index, SemInstr value);

#endif //STACK_SemInstr_H_
