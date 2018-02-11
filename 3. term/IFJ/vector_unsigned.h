/*
 * vector_unsigned.h
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

#ifndef VECTOR_unsigned_H_
#define VECTOR_unsigned_H_

//This is essentially an emulation of the C++ std::vector.
//Only some functionality was implemented.

#include <stddef.h>


#define VECTOR_unsigned_DEFAULT_SIZE 32

typedef struct{
	size_t size;
	size_t last;
	unsigned *array;
}Vector_unsigned;

int vector_unsigned_init(Vector_unsigned *vector);
int vector_unsigned_init_sized(Vector_unsigned *vector, int size);
void vector_unsigned_destroy(Vector_unsigned *vector);
//if error happens, the vector is still valid but the push didn't happen
int vector_unsigned_push_back(Vector_unsigned *vector, unsigned value);
unsigned vector_unsigned_pop_back(Vector_unsigned *vector);
unsigned* vector_unsigned_back(Vector_unsigned *vector);
size_t vector_unsigned_count(Vector_unsigned *vector);
unsigned* vector_unsigned_at(Vector_unsigned *vector, size_t index);
int vector_unsigned_insert_at(Vector_unsigned *vector, size_t index, unsigned value);

#endif //STACK_unsigned_H_
