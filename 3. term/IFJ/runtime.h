/*
 * runtime.h
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

#ifndef _IFJ_RUNTIMEH_
#define _IFJ_RUNTIMEH_

#include"graph.h"
#include"varstack.h"

extern Graph *inst;

void run_function(void);

void runtime_enviroment(void);

#endif
