/*
 * runtime.c
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

#include<stdlib.h>
#include"graph.h"
#include"varstack.h"
#include"inst_stack.h"
#include"error.h"

VarStack stackInstance = {.buffer = NULL};
VarStack *varStack = &stackInstance;

InstStack instInstance = {.buffer = NULL};
InstStack *instStack = &instInstance;

extern GraphTree *graphTree; //array
Graph *inst;

/* is only to be run by runtime_enviroment or by instructions */
void run_function(void)
{
	GraphTree *tree = &(graphTree[0]);
	inst = tree->start;
	bool next = true;

	while(inst != NULL)
	{
		next = inst->fPtr(inst->dest,inst->src);
		if(next)
			inst++;
		else
			inst = inst->rPtr;
	}
		
}

void runtime_enviroment(void)
{
	if(varstack_init(varStack) == EXIT_FAILURE)
		end_with_error(RET_INTERN);

	if(inst_init(instStack) == EXIT_FAILURE)
		end_with_error(RET_INTERN);

	Graph *dummy = NULL;
	inst_push(instStack,dummy-1); //after last return will land on dummy end end
	run_function();

	varstack_remove_variables(varStack,-1); //clears stack
	inst_destroy(instStack); //clears inst stack
	instStack=NULL;

}
