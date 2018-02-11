/*
 * dynamic_array.h
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

#ifndef __DYNAMICARRAYH
#define __DYNAMICARRAYH

#define START_ARRAY_SIZE 16

#include<stdlib.h>

typedef struct {
	size_t size;		//size of buffer
	size_t pos;		//position of write head
	char *buffer;	//buffer
} Array;

int array_init(Array *array);
int array_append_char(Array *array,char c);
int array_rewrite_pos(Array *array,char c,size_t pos);
void array_destroy(Array *array);
void array_add_to_last_written(Array *array,char c);

#endif
