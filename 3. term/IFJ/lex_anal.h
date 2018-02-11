/*
 * lex_anal.h
 * Autori: Radek Vit, Nikola Valesova
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

#ifndef _LEX_ANAL_H_
#define _LEX_ANAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include "token.h"
#include "dynamic_array.h"
#include "ial.h"
#include "error.h"


enum State {
	INIT,			// starting state
	IDENTIFICATOR,		// identificator
	NUMBER,			// number
	REAL_DOT,		// dot separating integer and decimal parts found
	REAL_DECIMAL,		// decimal part of a number
	REAL_EXP_SIGN,		// 'E' or 'e' representing exponent found
	REAL_PLUSMINUS,		// plus or minus found
	REAL_EXP,		// exponent part of a number
	STRING,			// string
	ESCAPE_SEQUENCE,	// '\' inside a string
	STRING_ERR,		//finising string literal after invalid escape sequence
	STRING_ERR_ESC,	//escape sequence inside invalid string
	STRING_HD1,			// first letter of expected \x number
	STRING_HD2,			// second letter of expected \x number
	COMMENT_MAYBE,		// possible comment
	COMMENT_LINE,		// comment to the end of line ('//')
	COMMENT_BLOCK,		// block comment (from '/*' to '*/') 
	COMMENT_END_MAYBE,	// possible end of a block comment
	LESS,			// '<'
	GREATER,		// '>'
	EQUAL,			// '='
	N_EQUAL		// '!'
	};


// reads a lexem and sends corresponding token
int get_token(FILE *input, Token *token);
// determines whether a string is a keyword
bool is_keyword(char *string,Token *token);

#endif   /* _LEX_ANAL_H_ */
