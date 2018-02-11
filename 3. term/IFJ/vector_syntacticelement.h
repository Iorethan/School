/*
 * vector_SyntacticElement.h
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

#ifndef VECTOR_SyntacticElement_H_
#define VECTOR_SyntacticElement_H_

//This is essentially an emulation of the C++ std::vector.
//Only some functionality was implemented.

#include <stddef.h>
#include "synt_anal_types.h"

#define VECTOR_SyntacticElement_DEFAULT_SIZE 32

typedef struct{
	size_t size;
	size_t last;
	SyntacticElement *array;
}Vector_SyntacticElement;

int vector_SyntacticElement_init(Vector_SyntacticElement *vector);
int vector_SyntacticElement_init_sized(Vector_SyntacticElement *vector, int size);
void vector_SyntacticElement_destroy(Vector_SyntacticElement *vector);
//if error happens, the vector is still valid but the push didn't happen
int vector_SyntacticElement_push_back(Vector_SyntacticElement *vector, SyntacticElement value);
SyntacticElement vector_SyntacticElement_pop_back(Vector_SyntacticElement *vector);
SyntacticElement* vector_SyntacticElement_back(Vector_SyntacticElement *vector);
size_t vector_SyntacticElement_count(Vector_SyntacticElement *vector);
SyntacticElement* vector_SyntacticElement_at(Vector_SyntacticElement *vector, size_t index);
int vector_SyntacticElement_insert_at(Vector_SyntacticElement *vector, size_t index, SyntacticElement value);

#endif //STACK_SyntacticElement_H_
