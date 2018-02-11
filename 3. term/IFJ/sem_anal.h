/*
 * sem_anal.h
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

#ifndef _SEM_ANAL_H_
#define _SEM_ANAL_H_

#include "sem_anal_types.h"
#include "synt_anal.h"

int analyze_program_parse_tree(ParseTree *parseTree);

#endif //_SEM_ANAL_H_
