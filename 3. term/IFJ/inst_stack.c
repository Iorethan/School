/*
 * dynamic_array.c
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

#include<stdlib.h>
#include"inst_stack.h"

/* initialises dynamic array */
int inst_init(InstStack *array)
{
	array->size = INST_SSIZE;
	array->pos = -1;

	if((array->buffer = malloc(INST_SSIZE * sizeof(Graph *))) == NULL)
	{
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

int inst_push(InstStack *a,Graph *v)
{
	a->pos += 1;
	if(a->pos >= a->size)
	{
		a->size = a->size << 1; //multiply by two
		Graph **old = a->buffer;
		a->buffer = realloc(a->buffer,a->size * sizeof(Graph *));
		if(a->buffer == NULL)
		{
			a->buffer = old;
			return EXIT_FAILURE;
		}
	}
	(a->buffer)[a->pos] = v;
	return EXIT_SUCCESS;
}

Graph *inst_pop(InstStack *a)
{
	a->pos -= 1;
	return (a->buffer)[a->pos + 1];
}

void inst_destroy(InstStack *a)
{
	a->size = 0;
	free(a->buffer);
}
