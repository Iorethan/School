/*
 * main.c
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


#include"error.h"
#include"lex_anal.h"
#include"synt_anal.h"
#include"sem_anal.h"
#include"runtime.h"


/* function opening the source file */
int open_file(int argc, char **argv, FILE **fr)
{
	if (argc == 2)						/* check number of arguments */
	{
		if ((*fr = fopen(argv[1], "r")) == NULL)
		{
			fprintf(stderr, "Error in opening the source file.\n");
			return RET_INTERN;
		}
		else 
			return RET_EOK;
	}

	else							/* invalid number of arguments */
	{
		fprintf(stderr, "Invalid number of arguments.\n");
		return RET_INTERN;
	}
	
	return RET_INTERN;
}


int main(int argc, char *argv[])
{
	FILE *fr;
	
	int errNum = open_file(argc, argv, &fr);		/* open source file */
	if (errNum != RET_EOK)					/* check if unsuccessful */
		return errNum;
	
	ParseTree parseTree;
	vector_Rule_init(&parseTree.rules);
	vector_Token_init(&parseTree.values);
	errNum = analyze_program_syntax(fr,&parseTree);
	fclose(fr);
	if (errNum != RET_EOK){
		end_with_error(errNum);
	}
	errNum = analyze_program_parse_tree(&parseTree);
	if (errNum != RET_EOK){
		end_with_error(errNum);
	}

	runtime_enviroment();

	free_memory();
	return RET_EOK;
}
