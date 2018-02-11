/*
 * graph.h
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

#ifndef _IFJ_GRAPHH_
#define _IFJ_GRAPHH_

#include<stdbool.h>

typedef bool (*InstrOp)(int, void*);

typedef struct graph Graph;
struct graph {
	bool (*fPtr)(int,void*);
	int dest;
	void *src;
	Graph *rPtr;
};

typedef struct graphTree
{
	Graph *start;
} GraphTree;

void graph_init(GraphTree *g);

typedef Graph *GraphPtr;

#endif
