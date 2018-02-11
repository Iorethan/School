/*
 * vector_Token.h
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

#ifndef VECTOR_Token_H_
#define VECTOR_Token_H_

//This is essentially an emulation of the C++ std::vector.
//Only some functionality was implemented.

#include <stddef.h>
#include "token.h"

#define VECTOR_Token_DEFAULT_SIZE 32

typedef struct{
	size_t size;
	size_t last;
	Token *array;
}Vector_Token;

int vector_Token_init(Vector_Token *vector);
int vector_Token_init_sized(Vector_Token *vector, int size);
void vector_Token_destroy(Vector_Token *vector);
//if error happens, the vector is still valid but the push didn't happen
int vector_Token_push_back(Vector_Token *vector, Token value);
Token vector_Token_pop_back(Vector_Token *vector);
Token* vector_Token_back(Vector_Token *vector);
size_t vector_Token_count(Vector_Token *vector);
Token* vector_Token_at(Vector_Token *vector, size_t index);
int vector_Token_insert_at(Vector_Token *vector, size_t index, Token value);

#endif //STACK_Token_H_
