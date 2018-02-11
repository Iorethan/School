/*
 * inst_support.c
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

#include "varstack.h"
#include "inst_support.h"
#include "error.h"
extern VarStack *varStack;

int compare(int dest, int *type)
{
	Var *var1,*var2;
	int x;
	if(dest == 0)
	{
		var1 = varstack_get_variable(varStack,1);
		var2 = varstack_get_variable(varStack,0);
	}
	else if (dest > 0)
	{
		var1 = varstack_get_variable(varStack,0);
		var2 = varstack_get_variable(varStack,dest);
		if(!var2->isinit)
			end_with_error(RET_UNINIT);
	}
	else
	{
		var1 = varstack_get_variable(varStack,-dest);
		if(!var1->isinit)
			end_with_error(RET_UNINIT);
		var2 = varstack_get_variable(varStack,0);
	}

	/* expecting to be used in expression, not checking for initialization */
	
	*type=var1->type;

	if(var1->type == TYPE_STRING)
	{
		x = compare_s(var1->string->data,var2->string->data);
	}	
	else if(var1->type == TYPE_NUMBER)
	{
		x = compare_i(var1->number,var2->number);
	}
	else
	{
		x = compare_d(var1->real,var2->real);
	}
	return x; // < 0 if smaller, > 0 if biger, 0 if equal
}

