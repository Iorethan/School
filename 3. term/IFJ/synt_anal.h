/*
 * synt_anal.h
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

#ifndef SYNT_ANAL_H_
#define SYNT_ANAL_H_

#include <stdio.h>
#include "synt_anal_types.h"
#include "vector_rule.h"
#include "vector_token.h"

typedef struct{
	Vector_Rule rules;
	Vector_Token values;
}ParseTree;

/**
 * @brief Analyzes the syntax of the program contained in progfile
 * @param progfile The stream from which the program will be read.
 * @param output The output of the parser. Expects both vectors to be initialized.
 * If it runs successfully, output will contain a sequence of rules used for parsing the program.
 * The rules are ordered as a leftmost derivation for everything except expressions,
 * which are ordered as a reversed rightmost derivation.
 * The values are ordered the same as the rules which produced the terminals carrying those values.
 * @return RET_EOK if the program is OK, an error otherwise
 */
int analyze_program_syntax(FILE *progfile, ParseTree *output);

#endif
