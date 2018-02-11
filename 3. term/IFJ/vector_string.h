/*
 * vector_String.h
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

#ifndef VECTOR_String_H_
#define VECTOR_String_H_

//This is essentially an emulation of the C++ std::vector.
//Only some functionality was implemented.

#include <stddef.h>
#include "sem_anal_types.h"

#define VECTOR_String_DEFAULT_SIZE 32

typedef struct{
	size_t size;
	size_t last;
	String *array;
}Vector_String;

int vector_String_init(Vector_String *vector);
int vector_String_init_sized(Vector_String *vector, int size);
void vector_String_destroy(Vector_String *vector);
//if error happens, the vector is still valid but the push didn't happen
int vector_String_push_back(Vector_String *vector, String value);
String vector_String_pop_back(Vector_String *vector);
String* vector_String_back(Vector_String *vector);
size_t vector_String_count(Vector_String *vector);
String* vector_String_at(Vector_String *vector, size_t index);
int vector_String_insert_at(Vector_String *vector, size_t index, String value);

#endif //STACK_String_H_
