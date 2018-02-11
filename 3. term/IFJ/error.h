/*
 * error.h
 * Autor: Nikola Valesova
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



/* preventing multiple inclusions */
#ifndef _ERROR_H_
#define _ERROR_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>



/* starting count of expected errors */
#define START_COUNT 50


/* enumerated type of all return values */ 
typedef enum 
{
	RET_EOK = 0,		/* no error */
	RET_LEX,			/* lexical error */
	RET_SYN,			/* syntactical error */
	RET_SEM_DEF,		/* semantic error, type 1 */
	RET_SEM_EXP,		/* semantic error, type 2 */
	RET_DERIVE,		/* deriving the data type of a variable */
	RET_SEM_OTHER,		/* semantic error, type 3 */
	RET_INPUT,		/* input error */
	RET_UNINIT,		/* uninitialized variable */
	RET_DIV,			/* division by zero */
	RET_RUN,			/* runtime error */
	RET_INTERN = 99,		/* intern error */
} Ret_value;

typedef enum
{
	ERR_EOK = 0,
	ERR_LEX,
	ERR_SYN,
	ERR_SEM_DEF,
	ERR_SEM_EXP,
	ERR_DERIVE,
	ERR_SEM_OTHER,
	ERR_INPUT,
	ERR_UINIT,
	ERR_DIV,
	ERR_RUN,
	ERR_INTERN
} Err_value;



/* prototypes of functions defined in error.c */
void print_error_message(Err_value errNum, int lineNumber);
void print_customized_message(const char *fmt, ...);
void free_memory();
void end_with_error(Ret_value errNum);

void error_message(char *message_format, int lineNumber, ...);

#endif	/* _ERROR_H_ */
