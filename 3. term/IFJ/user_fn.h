/*
 * user_fn.h
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

#ifndef _USER_FN_H_
#define _USER_FN_H_
#include "sem_anal_types.h"
#include "vector_funcparam.h"
#include "graph.h"

typedef struct{
	Vector_FuncParam params;
	SemVarType return_type;
	size_t first_inst_index;
}UserFn;

#endif //_FN_SIG_H_
