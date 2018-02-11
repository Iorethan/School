/*
 * varstack.c
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

#include"varstack.h"
#include<stdlib.h>
#include<string.h>
#include<assert.h>

int varstack_init(VarStack *s)
{
	s->size = VARSTACK_DEFSIZE;
	s->pos = 0; //first free item
	if ((s->buffer = malloc(sizeof(Var) * VARSTACK_DEFSIZE)) == NULL)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int varstack_insert_variable(VarStack *s,VarType type,VarValues value)
{
	if(s->pos >= s->size)
	{
		Var *old = s->buffer;
		s->size *= 2;
		if((s->buffer = realloc(s->buffer,sizeof(Var) * s->size)) == NULL)
		{
			s->buffer = old;
			return EXIT_FAILURE;
		}
	}
	Var *var = s->buffer + s->pos;

	var->type = type;
	var->isinit = true;

	if(type == TYPE_STRING){
		value.string->references++;
		var->string=value.string;
	}
	else if(type == TYPE_NUMBER)
		var->number = value.number;
	else if(type == TYPE_REAL)
		var->real = value.real;

	s->pos += 1;

	return EXIT_SUCCESS;
}
// TODO: rewrite for array
int varstack_insert_empty_variable(VarStack *s,VarType type)
{
	if(s->pos >= s->size)
	{
		Var *old = s->buffer;
		s->size *= 2;
		if((s->buffer = realloc(s->buffer,sizeof(Var) * s->size)) == NULL)
		{
			s->buffer = old;
			return EXIT_FAILURE;
		}
	}
	Var *var = s->buffer + s->pos;

	var->type = type;
	var->isinit = false;

	if (type == TYPE_STRING)
	{
		var->string = NULL;
	}
	else if (type == TYPE_NUMBER)
		var->number = 0;
	else if (type == TYPE_REAL)
		var->real = 0.0;

	s->pos += 1;

	return EXIT_SUCCESS;
}

void varstack_remove_variables(VarStack *s,int x)
{
	Var *var = s->buffer + s->pos;
	//check assumption about x never popping more than we have
	assert(x<0||(size_t)x<=s->pos);
	Var *end;
	if(x<0){
		end=s->buffer-1;
		s->pos=0;
	}
	else{
		end=var-x-1;
		s->pos-=x;
	}
	//better to double check
	assert(end>=s->buffer-1);
	while(--var>end){
		if(var->type==TYPE_STRING&&var->string){
			varstring_delete(var->string);
		}
	}
}

extern Var *varstack_get_variable(VarStack *s,int x);
extern void varstack_remove_one_string(VarStack *s);
extern void varstack_remove_one_init_string(VarStack *s);
extern void varstack_remove_nonstrings(VarStack *s, int count);

extern void varstring_delete(VarString *str);

void varstack_mark_initialized(VarStack *s,int x)
{
	(s->buffer + s->pos - x - 1)->isinit = true;
}

void varstack_destroy(VarStack *s)
{
	varstack_remove_variables(s,-1);
	free(s->buffer);
	s->buffer = NULL;
	s->size = 0;
	s->pos = 0;
}

void varstack_pop_variable_ext(VarStack *s,Var *var)
{
	Var *top = varstack_get_variable(s,0);
	*var = *top;

	s->pos -= 1;
}

void varstack_push_variable_ext(VarStack *s,Var *var)
{
	Var *top = s->buffer + s->pos;
	*top = *var;

	s->pos += 1;

}

int varstack_copy_variable(VarStack *s,Var *var)
{
	if(s->pos >= s->size)
	{
		Var *old = s->buffer;
		//relativize var to an offset into the buffer
		size_t offset=var-old;
		s->size *= 2;
		if((s->buffer = realloc(s->buffer,sizeof(Var) * s->size)) == NULL)
		{
			s->buffer = old;
			return EXIT_FAILURE;
		}
		//derelativize var
		var=s->buffer+offset;
	}
	Var *top = s->buffer + s->pos;

	*top = *var;

	if(var->type == TYPE_STRING) {
		var->string->references++;
	}

	s->pos += 1;

	return EXIT_SUCCESS;
}

