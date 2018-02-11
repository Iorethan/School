/*
 * varstack.h
 * Autor: Radek Vit
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

/* Contains data structures and functions needed for handling variable stack for
 * runtime of interpreted program.
 */

#ifndef _IFJ_VARSTACK_H_
#define _IFJ_VARSTACK_H_

#include<stdbool.h>
#include<stdlib.h>

#define VARSTACK_DEFSIZE 16

typedef enum {
	TYPE_STRING,
	TYPE_NUMBER,
	TYPE_REAL
} VarType;

typedef struct{
	int length; //keep string length signed
	unsigned references;
	char data[];
}VarString;

inline void varstring_delete(VarString *str){
	if(!--str->references){
		free(str);
	}
}

typedef struct varitem Var;
struct varitem {
	union {
		VarString *string;
		int number;
		double real;
	};
	VarType type;
	bool isinit;
};

typedef struct varstack
{
	size_t size;
	size_t pos;
	Var *buffer;
} VarStack;

typedef union varValues
{
		VarString *string;
		int number;
		double real;
} VarValues;

int varstack_init(VarStack *s); 

/* inserts variable at top of stack; returns EXIT_FAILURE if unsuccesful */
int varstack_insert_variable(VarStack *s,VarType type,VarValues value);

int varstack_insert_empty_variable(VarStack *s,VarType type);

/* removes x variables from stack; removes all if x < 0 */
void varstack_remove_variables(VarStack *s,int x);

inline void varstack_remove_one_string(VarStack *s){
	size_t npos=--s->pos;
	Var *var=s->buffer+npos;
	if(var->string){
		varstring_delete(var->string);
	}
}

inline void varstack_remove_one_init_string(VarStack *s){
	varstring_delete((s->buffer+(--s->pos))->string);
}

inline void varstack_remove_nonstrings(VarStack *s, int count){
	s->pos-=count;
}

/* returns pointer to variable x from top of stack; does not check if
 * variable exists, calling it on empty or smaller than x+1 stack causes
 * segfault; puts correct values to non-NULL pointers */
inline Var *varstack_get_variable(VarStack *s,int x){
	return s->buffer+s->pos-x-1;
}

void varstack_destroy(VarStack *s);

/* copies variable from top to external variable */
void varstack_pop_variable_ext(VarStack *s, Var *var);

/* copies variable back to top from external variable */
void varstack_push_variable_ext(VarStack *s,Var *Var);

/* copies variable var to top of stack */
int varstack_copy_variable(VarStack *s,Var *var);

#endif
