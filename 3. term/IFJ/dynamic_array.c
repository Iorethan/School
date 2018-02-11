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
#include<string.h>
#include"dynamic_array.h"
#include"error.h"

/* initialises dynamic array */
int array_init(Array *array)
{
	array->size = START_ARRAY_SIZE;
	array->pos = 0;

	if((array->buffer = malloc(array->size)) == NULL)
	{
		return EXIT_FAILURE;
	}

	memset(array->buffer,'\0',array->size); //initialising with 0
	
	return EXIT_SUCCESS;
}

/* appends character to string in buffer */
int array_append_char(Array *array, char c)
{
	if(array->pos+1 >= array->size) //buffer too small for 0 terminated string
	{
		array->size = array->size << 1; //multiplying size by two

		if((array->buffer = realloc(array->buffer,array->size)) == NULL)
		{ //NOT restoring original size of buffer
			return EXIT_FAILURE;
		}

		memset((array->buffer)+(array->pos)+1,'\0',array->size/2);
		//initialising new space with 0
	}

	array->buffer[array->pos] = c;
	array->pos += 1;
	return EXIT_SUCCESS;
}

/* rewrites char on position pos if possible */
int array_rewrite_pos(Array *array, char c, size_t pos)
{
	if(pos >= array->size)
		return EXIT_FAILURE;

	array->buffer[pos] = c;
	return EXIT_SUCCESS;
}

/* frees allocated space and sets everything to 0 */
void array_destroy(Array *array)
{
	array->size = 0;
	array->pos = 0;
	free(array->buffer);
	array->buffer = NULL;
}

void array_add_to_last_written(Array *array,char c)
{
	if(array->pos == 0)
		return;
	
	array->buffer[array->pos - 1] += c;
}
