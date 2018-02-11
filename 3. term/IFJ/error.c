/*
 * error.c
 * Autor: Nikola Valesova
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


#include "error.h"
#include "inst_stack.h"
#include "varstack.h"
#include "resources.h"
#include "ial.h"
#include "assert.h"
/* all extern global variables used for memory free */
extern InstStack *instStack;
extern VarStack *varStack;


/* function that writes out an error warning to stderr */
void warning(Err_value errNum, int lineNumber)
{
	static char *message[] = {
		"",
		"Error in lexical analysis on line %d.",
		"Error in syntactic analysis on line %d.",
		"A semantic error (an undefined function / variable...) on line %d.",
		"A semantic error in type compatibility on line %d.",
		"Error in deriving the data type of a variable on line %d.",
		"An unspecified semantic error on line %d.",
		"A runtime error in reading a numeric input.",
		"A runtime error in use of an uninitialized variable.",
		"A runtime error in division by zero.",
		"An unspecified runtime error.",
		"An internal error occurred."
	};
	
	fprintf(stderr,message[errNum],lineNumber);
	fprintf(stderr,"\n");
	return;
}

/* function that writes out a customized error warning to stderr */
void print_customized_message(const char *fmt, ...)	// add terminal x nonterminal
{
    va_list args;
    va_start(args, fmt);

    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n" );

    va_end(args);
	return;
}

/* function that frees all allocated memory */
void free_memory()			// TODO: add what you need to be freed
{
/* interpret */
	if(instStack){
		inst_destroy(instStack);
	}
	varstack_destroy(varStack);
	//semantic analysis
	if(graphTree){
		free(program);
		free(graphTree);
		size_t constant_count=vector_SemConst_count(&constant_store);
		for(size_t i=0; i<constant_count; i++){
			SemConst *constant=vector_SemConst_at(&constant_store, i);
			if(constant->type==TYPE_STRING){
				assert(constant->string->references==1);
				varstring_delete(constant->string);
			}
		}
		vector_SemConst_destroy(&constant_store);
	}
	return;
}

/* function that frees all allocated memory and ends the program */
void end_with_error(Ret_value errNum)
{
	Err_value errMsg = (errNum == RET_INTERN) ? ERR_INTERN : errNum;
	warning(errMsg,0);
	free_memory();
	exit(errNum);
}

void error_message(char *message_format, int lineNumber, ...){
	va_list args;
	va_start(args, lineNumber);
	vfprintf(stderr, message_format, args);
	va_end(args);
	fprintf(stderr, "\nOn line %d\n",lineNumber);
	//TODO: fill in.
}
