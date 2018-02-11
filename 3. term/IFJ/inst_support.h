/*
 * inst_support.h
 * Autor: Radek Vit
 * Predmet: Formalni jazyky a prekladace
 * Projekt: Implementace interpretu jazyka IFJ15
 * Varianta zadani: a/2/II
 * Datum: 13. 11. 2015
 * Prelozeno: gcc 4.8
 * Tomas Zencak xzenca00
 * Ondrej Vales xvales03
 * Nikola Valesova xvales02
 * Radek Vit xvitra00
 */

#ifndef _IFJ_INSTS_
#define _IFJ_INSTS_

#include<string.h>

int compare(int dest, int *type); //compares top two from stack

#define abs(x) (x >= 0 ? x : -x)

#define add_i(x,y) ( x+y )
#define sub_i(x,y) ( x+y )
#define div_i(x,y) ( x/y )
#define mul_i(x,y) ( x*y )

#define add_d(x,y) ( x+y )
#define sub_d(x,y) ( x-y )
#define div_d(x,y) ( x/y )
#define mul_d(x,y) ( x*y )

/* returns - if x is smaller than y, + if bigger, 0 if equal */
#define compare_s(x,y) strcmp(x,y)
#define compare_i(x,y) (x == y) ? 0 : ((x < y) ? -1 : 1)
#define compare_d(x,y) (x == y) ? 0 : ((x < y) ? -1 : 1)

#endif
