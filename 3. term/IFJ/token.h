/*
 * token.h
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

#ifndef _TOKEN_H_
#define _TOKEN_H_

#include"ial.h"

#define MAX_KWORD 6

typedef enum {
	LEX_IAUTO,		// keyword auto
	LEX_ICIN,		// keyword cin
	LEX_ICOUT,		// keyword cout
	LEX_IDOUBLE,	// keyword double
	LEX_IELSE,		// keyword else
	LEX_IFOR,		// keyword for
	LEX_IIF,		// keyword if
	LEX_IINT,		// keyword int
	LEX_IRETURN,	// keyword return
	LEX_ISTRING,	// keyword string
	LEX_IWHILE,		// keyword while
	LEX_IDO,			// keyword do
	LEX_NUMBER,		// number
	LEX_REAL_NUMBER,// fp number
	LEX_STRING_LITERAL,	//string literal
	LEX_ID,			// identificator
	LEX_PLUS,		// '+'
	LEX_MINUS,		// '-'
	LEX_MULTIPLY,	// '*'
	LEX_DIVIDE,		// '/'
	LEX_COMMA,		// ','
	LEX_LPARENTHESIS,	// '('
	LEX_RPARENTHESIS, // ')'
	LEX_LBRACE,		// '{'
	LEX_RBRACE,		// '}'
	LEX_SEMICOLON,	// ';'
	LEX_LESS,		// '<'
	LEX_LESSEQ,		// '<='
	LEX_GREATER,	// '>'
	LEX_GREATEREQ,	// '>='
	LEX_EQUAL,		// '=='
	LEX_ASSIGN,		// '='
	LEX_NOTEQUAL,	// '!='
	LEX_STREAM_EXTRACT,	// '>>'
	LEX_STREAM_INSERT,		// '<<'
	LEX_EOF,			// EOF
	LEXEME_TYPE_COUNT	//count of token types.
} TokenType;

typedef union {
	char 	*IDName;
	char 	*string;
	int	number;
	double 	real;
} Attribute;

typedef struct {
	TokenType type;
	union {
		char 	*IDName;
		char 	*string;
		int	number;
		double 	real;
	};
	unsigned int line;
	unsigned int position;
} Token;

#endif  /* _TOKEN_H_ */
