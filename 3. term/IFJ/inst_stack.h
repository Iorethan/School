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

#ifndef __INSTSTACKH
#define __INSTSTACKH

#define INST_SSIZE 16

#include<stdlib.h>
#include "graph.h"


typedef struct {
	size_t size;		//size of buffer
	size_t pos;		//position of write head
	Graph **buffer;	//buffer
} InstStack;

extern InstStack *instStack;

int inst_init(InstStack *a);
int inst_push(InstStack *a,Graph *v);
Graph *inst_pop(InstStack *a);
void inst_destroy(InstStack *array);

#endif
