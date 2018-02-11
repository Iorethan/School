/*
 * instructions.c
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

#include"instructions.h"
#include"varstack.h"
#include"error.h"
#include"inst_support.h"
#include"inst_stack.h"
#include"ial.h"
#include"runtime.h"
#include"dynamic_array.h"
#include<ctype.h>

extern VarStack *varStack;
extern GraphTree *graphTree; //array

/***** I/O instructions *****/
bool inst_output_number(int dest, void *src)
{
	if (src != NULL) //output from src
		printf("%d",*(int *)src);
	else //output from variable dest
	{
		Var *var = varstack_get_variable(varStack,dest);
		if(var->isinit)
			printf("%d",var->number);
		else
			end_with_error(RET_UNINIT);
	}

	return true;
}

bool inst_output_real(int dest, void *src)
{
	if (src != NULL) //output from src
		printf("%g",*(double *)src);
	else //output from variable dest
	{
		Var *var = varstack_get_variable(varStack,dest);
		if(var->isinit)
			printf("%g",var->real);
		else
			end_with_error(RET_UNINIT);
	}

	return true;
}

bool inst_output_string(int dest, void *src)
{
	if (src != NULL) //output from src
		printf("%s",(*(VarString**)src)->data);
	else //output from variable dest
	{
		Var *var = varstack_get_variable(varStack,dest);
		if(var->isinit)
			printf("%s",var->string->data);
		else
			end_with_error(RET_UNINIT);
	}

	return true;
}

bool inst_input_number(int dest, void *src)
{
	(void)src;
	int c, num = 0;
	do //discarding whitespaces
		c = getchar();
	while (isspace(c));
	
	if(!isdigit(c)){
		end_with_error(RET_INPUT);
	}
	while (isdigit(c)) //converting number
	{
		num *= 10;
		num += c - '0';
		c = getchar();
	}
	
	if(!isspace(c))
		ungetc(c,stdin);

	Var *var = varstack_get_variable(varStack, dest);
	var->number = num;
	var->isinit = true;

	
	return true;
}

bool inst_input_real(int dest, void *src)
{
	(void)src;
	int c;
	bool whole = false, frac = false, exp = false;
	Array tmpI;	
	Array *tmp = &tmpI;
	if (array_init(tmp) == EXIT_FAILURE)
		end_with_error(RET_INTERN);
	
	do //discarding whitespaces
		c = getchar();
	while (isspace(c));
	
	while (isdigit(c)) //whole part
	{
		if (array_append_char(tmp, c) == EXIT_FAILURE)
			end_with_error(RET_INTERN);
		whole = true;
		c = getchar();
	}
	
	if (c == '.') //decimal part
	{
		if (array_append_char(tmp, c) == EXIT_FAILURE)
			end_with_error(RET_INTERN);
		c = getchar();		
		while (isdigit(c))
		{
			if (array_append_char(tmp, c) == EXIT_FAILURE)
				end_with_error(RET_INTERN);
			frac = true;
			c = getchar();
		}
		if (!frac)	
		{
			array_destroy(tmp);
			end_with_error(RET_INPUT);
		}
	}
	
	if (c == 'e' || c == 'E') //exponent
	{
		if (array_append_char(tmp, c) == EXIT_FAILURE)
			end_with_error(RET_INTERN);
		c = getchar();
		
		if (c == '+' || c == '-')
		{
			if (array_append_char(tmp, c) == EXIT_FAILURE)
				end_with_error(RET_INTERN);
			c = getchar();
		}

		while (isdigit(c))
		{
			if (array_append_char(tmp, c) == EXIT_FAILURE)
				end_with_error(RET_INTERN);
			exp = true;
			c = getchar();
		}
		if (!exp)	
		{
			array_destroy(tmp);
			end_with_error(RET_INPUT);
		}
	}
	ungetc(c, stdin);
	if (whole && (frac || exp)) //valid real
	{
		Var *var = varstack_get_variable(varStack, dest);
		var->real = strtod(tmp->buffer,NULL);
		var->isinit = true;
		array_destroy(tmp);
	}		
	else
	{
		array_destroy(tmp);
		end_with_error(RET_INPUT);
	}
	return true;
}

bool inst_input_string(int dest, void *src)
{
	(void)src;
	Array tmpI;
	Array *tmp = &tmpI;
	int c;
	if (array_init(tmp) == EXIT_FAILURE)
		end_with_error(RET_INTERN);
	
	do //discarding whitespaces
		c = getchar();
	while (isspace(c));
	while (!(isspace(c) || c == EOF))
	{
		if (array_append_char(tmp, c) == EXIT_FAILURE)
			end_with_error(RET_INTERN);
		c = getchar();
	}
	
	Var *var = varstack_get_variable(varStack, dest);
	if(var->string){
		varstring_delete(var->string);
	}
	VarString *nstr=malloc(sizeof(VarString)+tmp->pos+1);
	if(!nstr){
		array_destroy(tmp);
		end_with_error(RET_INTERN);
	}
	strcpy(nstr->data, tmp->buffer);
	nstr->length=tmp->pos;
	nstr->references=1;
	array_destroy(tmp);
	var->string = nstr;
	var->isinit = true;	
	return true;	
}
/***** *****/

/***** CONTROL instructions *****/
bool inst_add_variable(int dest,void *src)
{
	int succ;
	VarValues val;

	if(src != NULL) //inserting initialised variable
	{
		if(dest == TYPE_NUMBER)
		{
			val.number = *(int *)src;
			succ = varstack_insert_variable(varStack,dest,val);
		}
		else if(dest == TYPE_REAL)
		{
			val.real = *(double *)src;
			succ = varstack_insert_variable(varStack,dest,val);
		}
		else //string
		{
			val.string = *(VarString **)src;
			succ = varstack_insert_variable(varStack,dest,val);
		}
	}
	else //unitialised variable
		succ = varstack_insert_empty_variable(varStack,dest);

	if(succ == EXIT_FAILURE)
		end_with_error(RET_INTERN);

	return true;
}

bool inst_copy_variable(int dest, void *src)
{
	(void)src;
	int succ;

	Var *var = varstack_get_variable(varStack,dest);
	if(!var->isinit) //uninitialised variables cannot be copied, expecting value
		end_with_error(RET_UNINIT);

	//CAUTION!!! var may become invalid here so DO NOT USE AFTER THIS STATEMENT!!!!!
	succ = varstack_copy_variable(varStack,var);
	if(succ != EXIT_SUCCESS)
		end_with_error(RET_INTERN);

	return true;
}

bool inst_pop_variables(int dest,void *src)
{
	(void)src;
	varstack_remove_variables(varStack,dest);
	return true;
}

bool inst_swap(int dest, void *src)
{
	(void)dest; (void)src;

	Var var0,var1;
	varstack_pop_variable_ext(varStack,&var0);
	varstack_pop_variable_ext(varStack,&var1);
	varstack_push_variable_ext(varStack,&var0);
	varstack_push_variable_ext(varStack,&var1);

	return true;
}

bool inst_assign(int dest,void *src)
{
	(void)src;

	Var *varsrc,*vardest;
	/*assuming varsrc came from an expression and is initialised */
	varsrc = varstack_get_variable(varStack,0);
	vardest = varstack_get_variable(varStack,dest);

	vardest->isinit = true;
	if(	vardest->type == TYPE_NUMBER)
	{
		vardest->number = varsrc->number;
		varstack_remove_nonstrings(varStack, 1);
	}
	else if (vardest->type == TYPE_REAL)
	{
		vardest->real = varsrc->real;
		varstack_remove_nonstrings(varStack, 1);
	}
	else
	{
		if(vardest->string){
			varstring_delete(vardest->string);
		}
		vardest->string = varsrc->string;
		varsrc->string->references++;
		varstack_remove_one_init_string(varStack);
	}

	return true;
}

bool inst_assign_const(int dest,void *src)
{
	Var *var = varstack_get_variable(varStack,dest);
	var->isinit = true;
	/* constant is always of same type as target variable */
	if(var->type == TYPE_STRING)
	{
		var->string=*(VarString**)src;
		var->string->references++;
	}
	else if (var->type == TYPE_NUMBER)
	{
		var->number = *(int *)src;
	}
	else
	{
		var->real = *(double *)src;
	}
	return true;
}

bool inst_run_user_fc(int dest, void *src)
{
	(void)src;
	if(inst_push(instStack,inst) == EXIT_FAILURE)
		end_with_error(RET_INTERN);
	GraphTree *tree = &(graphTree[dest]);
	inst = tree->start;
	return true;
}

bool inst_run_builtin_fc(int dest,void *src)
{
	(void)src;
	Var *var1,*var2,*var3;
	VarValues ret;
	/* last parameter is on top of stack, parameters are of correct type */
	switch(dest)
	{ //TODO: create enum
		case 0: //length
			var1 = varstack_get_variable(varStack,0);
			ret.number = var1->string->length;
			varstack_remove_one_init_string(varStack);
			varstack_insert_variable(varStack,TYPE_NUMBER,ret); //no need to check
			break;
		case 1: //substr
			var1 = varstack_get_variable(varStack,2); //string
			var2 = varstack_get_variable(varStack,1); //pos
			var3 = varstack_get_variable(varStack,0); //length
			/* smaller of the two */
			int length = (var3->number < (var1->string->length - var2->number)) ?
			var3->number : (var1->string->length - var2->number);
			if(length < 0 || var2->number < 0)
			{
				end_with_error(RET_RUN);
			}
			ret.string=malloc(sizeof(VarString)+length+1);
			if(!ret.string){
				end_with_error(RET_INTERN);
			}
			ret.string->length=length;
			ret.string->references=0;

			ret.string->data[0]='\0';
			memcpy(ret.string->data, var1->string->data+var2->number, length);
			ret.string->data[length] = '\0';
			varstack_remove_nonstrings(varStack, 2);
			varstack_remove_one_init_string(varStack);
			if(varstack_insert_variable(varStack,TYPE_STRING,ret)
				!=EXIT_SUCCESS )
			{
				free(ret.string);
				end_with_error(RET_INTERN);
			}
			break;
		case 2: //concat
			var1 = varstack_get_variable(varStack,1);
			var2 = varstack_get_variable(varStack,0);
			ret.string = malloc(sizeof(VarString)+var1->string->length+var2->string->length+1);
			if(!ret.string){
				end_with_error(RET_INTERN);
			}
			ret.string->references=0;
			ret.string->length=var1->string->length+var2->string->length;
			//copy the first string
			memcpy(ret.string->data, var1->string->data, var1->string->length+1);
			//and append the second one
			strcat(ret.string->data, var2->string->data);
			varstack_remove_one_init_string(varStack);
			varstack_remove_one_init_string(varStack);
			if(varstack_insert_variable(varStack,TYPE_STRING,ret)
				!=EXIT_SUCCESS)
			{
				varstring_delete(ret.string);
				end_with_error(RET_INTERN);
			}
			break;
		case 3: //find
			var1 = varstack_get_variable(varStack,1);
			var2 = varstack_get_variable(varStack,0);
			ret.number =
			find(var1->string->data,var2->string->data,var1->string->length,var2->string->length);
			varstack_remove_one_init_string(varStack);
			varstack_remove_one_init_string(varStack);
			varstack_insert_variable(varStack,TYPE_NUMBER,ret);
			break;
		case 4: //sort
			var1 = varstack_get_variable(varStack,0);
			//if the only reference to this string is on top of the stack
			//just change that string and leave it there
			if(var1->string->references==1){
				heapsort(var1->string->data,var1->string->length);
			}
			else{
				VarString *sorted=malloc(sizeof(VarString)+var1->string->length+1);
				if(!sorted){
					end_with_error(RET_INTERN);
				}
				sorted->length=var1->string->length;
				sorted->references=1;
				memcpy(sorted->data, var1->string->data, var1->string->length+1);
				varstring_delete(var1->string);
				var1->string=sorted;
				heapsort(sorted->data, sorted->length);
			}
			break;
	}
	return true;	
}

bool inst_condition(int dest,void *src)
{
	(void)dest; (void)src;

	int value;
	Var *var = varstack_get_variable(varStack,(dest == -1) ? 0 : dest);
	if(dest != -1 && !var->isinit)
		end_with_error(RET_UNINIT);
		value = var->number;	
	if(dest==-1){
		varstack_remove_nonstrings(varStack, 1);
	}
	
	return (value == 0) ? false : true;

}

bool inst_jmp(int dest,void *src)
{
	(void)dest;
	(void)src;
	return false;
}

bool inst_ret(int dest,void *src)
{
	(void)src;
	Var var;
	varstack_pop_variable_ext(varStack,&var); //non-destructive pop
	varstack_remove_variables(varStack,dest-1);
	varstack_push_variable_ext(varStack,&var); //reinserting ret variable to top
	inst = inst_pop(instStack); //return instruction
	return true;
}

bool inst_nop(int dest,void *src)
{
	(void)dest; (void)src;
	return true;
}

bool inst_error(int dest,void *src)
{
	(void)dest; (void)src;

	end_with_error(dest);
	return true;
}
/***** *****/

/***** ARITHMETIC instructions *****/
bool inst_mult(int dest,void*src)
{
	(void)src;
	bool top = dest == 0 ? true : false; //taking two top variables, popping two
	dest = abs(dest); //in case of reverse order

	Var *var1,*var2;
	VarValues val;
	int type = TYPE_NUMBER;
	var1 = varstack_get_variable(varStack,(dest == 0 ? 1 : dest));
	var2 = varstack_get_variable(varStack,0);

	if(!top && !(var1->isinit)) //uninitialised variable, not possible with top
		end_with_error(RET_UNINIT);

	if(var1->type == TYPE_NUMBER)
	{
		val.number = var1->number * var2->number;
	}
	else
	{
		type = TYPE_REAL;
		val.real = var1->real * var2->real;
	}
	
	varstack_remove_nonstrings(varStack, top?2:1);
	varstack_insert_variable(varStack,type,val);

	return true;
}

bool inst_mult_const_i(int dest,void *src)
{
	(void)dest;
// never popping, rewriting variable value; should not be done with variables,
// does not set isinit
	Var *var = varstack_get_variable(varStack,0);
	var->number = var->number * *(int *)src;

	return true;
}

bool inst_mult_const_d(int dest, void *src)
{
	(void)dest;

	Var *var = varstack_get_variable(varStack,0);
	var->real = var->real * *(double *)src;

	return true;
}

bool inst_div(int dest,void*src)
{
	(void)src;
	bool top = dest == 0 ? true : false; //taking two from top
	bool reverse = dest > 0 ? true : false; // dest > 0 ? top/var ? var/top
	dest = abs(dest);

	Var *var1,*var2;
	VarValues val;
	int type = TYPE_NUMBER;
	var1 = varstack_get_variable(varStack,(dest == 0 ? 1 : dest));
	var2 = varstack_get_variable(varStack,0);
	
	if(!top && !(var1->isinit))
		end_with_error(RET_UNINIT);

	if(reverse)
	{
		Var *tmp = var1;
		var1 = var2;
		var2 = tmp;
	}

	if(var2->type == TYPE_NUMBER)
	{
		if(var2->number == 0)
		{
			end_with_error(RET_DIV);
			return true;
		}
		val.number = var1->number / var2->number;
	}
	else
	{
		if(var2->real == 0.0)
		{
			end_with_error(RET_DIV);
			return true;
		}
		type = TYPE_REAL;
		val.real = var1->real / var2->real;
	}
	varstack_remove_nonstrings(varStack, top?2:1);
	varstack_insert_variable(varStack,type,val);

	return true;
}

bool inst_div_const_i(int dest,void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	if(dest >= 0)
	{
		if(*(int *)src == 0)
			end_with_error(RET_DIV);

		var->number = var->number / *(int *)src;
	}
	else
	{
		if(var->number == 0)
			end_with_error(RET_DIV);
		var->number = *(int *)src / var->number;
	}

	return true;
}

bool inst_div_const_d(int dest,void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	if(dest >= 0)
	{
		if(*(double *)src == 0.0)
			end_with_error(RET_DIV);

		var->real = var->real / *(double *)src;
	}
	else
	{
		if(var->real == 0.0)
			end_with_error(RET_DIV);
		var->real = *(double *)src / var->real;
	}
	return true;
}

bool inst_add(int dest,void*src)
{
	(void)src;
	bool top = dest == 0 ? true : false; //taking two from top
	dest = abs(dest);


	Var *var1,*var2;
	VarValues val;
	int type = TYPE_NUMBER;
	var1 = varstack_get_variable(varStack,(dest == 0 ? 1 : dest));
	var2 = varstack_get_variable(varStack,0);

	if(!top && !(var1->isinit))
		end_with_error(RET_UNINIT);

	if(var1->type == TYPE_NUMBER)
	{
		val.number = var1->number + var2->number;
	}
	else
	{
		type = TYPE_REAL;
		val.real = var1->real + var2->real;
	}
	varstack_remove_nonstrings(varStack, top?2:1);
	varstack_insert_variable(varStack,type,val);

	return true;
}

bool inst_add_const_i(int dest,void *src)
{
	(void)dest;

	Var *var = varstack_get_variable(varStack,0);
	var->number = var->number + *(int *)src;

	return true;
}

bool inst_add_const_d(int dest, void *src)
{
	(void)dest;

	Var *var = varstack_get_variable(varStack,0);
	var->real = var->real + *(double *)src;

	return true;
}

bool inst_sub(int dest,void*src)
{
	(void)src;
	bool top = dest == 0 ? true : false; //taking two from top
	bool reverse = dest > 0 ? true : false; // dest > 0 ? top/var ? var/top
	dest = abs(dest);

	Var *var1,*var2;
	VarValues val;
	int type = TYPE_NUMBER;
	var1 = varstack_get_variable(varStack,(dest == 0 ? 1 : dest));
	var2 = varstack_get_variable(varStack,0);

	if(!top && !(var1->isinit))
		end_with_error(RET_UNINIT);

	if(reverse)
	{
		Var *tmp = var1;
		var1 = var2;
		var2 = tmp;
	}

	if(var1->type == TYPE_NUMBER)
	{
		val.number = var1->number - var2->number;
	}
	else
	{
		type = TYPE_REAL;
		val.real = var1->real - var2->real;
	}
	varstack_remove_nonstrings(varStack, top?2:1);
	varstack_insert_variable(varStack,type,val);

	return true;
}

bool inst_sub_const_i(int dest,void *src)
{
	(void)dest;

	Var *var = varstack_get_variable(varStack,0);

	var->number = var->number - *(int *)src;
	var->number = (dest >= 0) ? var->number : -(var->number);
	
	return true;
}

bool inst_sub_const_d(int dest, void *src)
{
	(void)dest;

	Var *var = varstack_get_variable(varStack,0);
	var->real = var->real - *(double *)src;
	var->real = (dest >= 0) ? var->real : -(var->real);

	var->type = TYPE_REAL;

	return true;
}
/***** *****/

/***** LOGIC instructions *****/
bool inst_logic_eq(int dest,void *src)
{
	(void)dest; (void)src;
	VarValues val;
	int x;
	int type;

	x = compare(dest, &type);

	x = (x == 0) ? ITRUE : IFALSE;

	val.number = x;
	if(type==TYPE_STRING){
		varstack_remove_one_init_string(varStack);
		if(dest==0){
			varstack_remove_one_init_string(varStack);
		}
	}
	else{
		varstack_remove_nonstrings(varStack, dest==0?2:1);
	}
	varstack_insert_variable(varStack, TYPE_NUMBER, val);
	
	return true;
}

bool inst_logic_neq(int dest,void *src)
{
	(void)dest; (void)src;
	VarValues val;
	int x;
	int type;

	x = compare(dest, &type);

	x = (x != 0) ? ITRUE : IFALSE;

	val.number = x;
	if(type==TYPE_STRING){
		varstack_remove_one_init_string(varStack);
		if(dest==0){
			varstack_remove_one_init_string(varStack);
		}
	}
	else{
		varstack_remove_nonstrings(varStack, dest==0?2:1);
	}
	varstack_insert_variable(varStack, TYPE_NUMBER, val);
	
	return true;
}

bool inst_logic_le(int dest,void *src)
{
	(void)dest; (void)src;
	VarValues val;
	int x;
	int type;

	x = compare(dest, &type);

	x = (x < 0) ? ITRUE : IFALSE;

	val.number = x;
	if(type==TYPE_STRING){
		varstack_remove_one_init_string(varStack);
		if(dest==0){
			varstack_remove_one_init_string(varStack);
		}
	}
	else{
		varstack_remove_nonstrings(varStack, dest==0?2:1);
	}
	varstack_insert_variable(varStack, TYPE_NUMBER, val);
	
	return true;
}

bool inst_logic_leeq(int dest,void *src)
{
	(void)dest; (void)src;
	VarValues val;
	int x;
	int type;

	x = compare(dest, &type);

	x = (x <= 0) ? ITRUE : IFALSE;

	val.number = x;
	if(type==TYPE_STRING){
		varstack_remove_one_init_string(varStack);
		if(dest==0){
			varstack_remove_one_init_string(varStack);
		}
	}
	else{
		varstack_remove_nonstrings(varStack, dest==0?2:1);
	}
	varstack_insert_variable(varStack, TYPE_NUMBER, val);
	
	return true;
}

bool inst_logic_gr(int dest,void *src)
{
	(void)dest; (void)src;
	VarValues val;
	int x;
	int type;

	x = compare(dest, &type);

	x = (x > 0) ? ITRUE : IFALSE;

	val.number = x;
	if(type==TYPE_STRING){
		varstack_remove_one_init_string(varStack);
		if(dest==0){
			varstack_remove_one_init_string(varStack);
		}
	}
	else{
		varstack_remove_nonstrings(varStack, dest==0?2:1);
	}
	varstack_insert_variable(varStack, TYPE_NUMBER, val);
	
	return true;
}

bool inst_logic_greq(int dest,void *src)
{
	(void)dest; (void)src;
	VarValues val;
	int x;
	int type;

	x = compare(dest, &type);

	x = (x >= 0) ? ITRUE : IFALSE;

	val.number = x;
	if(type==TYPE_STRING){
		varstack_remove_one_init_string(varStack);
		if(dest==0){
			varstack_remove_one_init_string(varStack);
		}
	}
	else{
		varstack_remove_nonstrings(varStack, dest==0?2:1);
	}
	varstack_insert_variable(varStack, TYPE_NUMBER, val);
	
	return true;
}

bool inst_logic_gr_const_s(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp = compare_s(var->string->data,(*(VarString **)src)->data);
	comp = dest >= 0 ? comp : -comp;
	b = comp > 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_one_init_string(varStack);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_le_const_s(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp = compare_s(var->string->data,(*(VarString **)src)->data);
	comp = dest >= 0 ? comp : -comp;
	b = comp < 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_one_init_string(varStack);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_greq_const_s(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp = compare_s(var->string->data,(*(VarString **)src)->data);
	comp = dest >= 0 ? comp : -comp;
	b = comp >= 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_one_init_string(varStack);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_leeq_const_s(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp = compare_s(var->string->data,(*(VarString **)src)->data);
	comp = dest >= 0 ? comp : -comp;
	b = comp <= 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_one_init_string(varStack);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_eq_const_s(int dest, void *src)
{
	(void)dest;
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp = compare_s(var->string->data,(*(VarString**)src)->data);
	b = comp == 0 ? true : false; 
	val.number = b ? 1 : 0;
	varstack_remove_one_init_string(varStack);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_neq_const_s(int dest, void *src)
{
	(void)dest;
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp = compare_s(var->string->data,(*(VarString**)src)->data);
	b = comp != 0 ? true : false;	 
	val.number = b ? 1 : 0;
	varstack_remove_one_init_string(varStack);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_gr_const_i(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_i(var->number,*(int *)src);
	comp = dest >= 0 ? comp : -comp;
	b = comp > 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_le_const_i(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_i(var->number,*(int *)src);
	comp = dest >= 0 ? comp : -comp;
	b = comp < 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_greq_const_i(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_i(var->number,*(int *)src);
	comp = dest >= 0 ? comp : -comp;
	b = comp >= 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_leeq_const_i(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_i(var->number,*(int *)src);
	comp = dest >= 0 ? comp : -1*comp;
	b = comp <= 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_eq_const_i(int dest, void *src)
{
	(void)dest;
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_i(var->number,*(int *)src);
	b = comp == 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_neq_const_i(int dest, void *src)
{
	(void)dest;
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_i(var->number,*(int *)src);
	b = comp != 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_gr_const_d(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_d(var->real,*(double *)src);
	comp = dest >= 0 ? comp : -1*comp;
	b = comp > 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_le_const_d(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_d(var->real,*(double *)src);
	comp = dest >= 0 ? comp : -1*comp;
	b = comp < 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_greq_const_d(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_d(var->real,*(double *)src);
	comp = dest >= 0 ? comp : -1*comp;
	b = comp >= 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_leeq_const_d(int dest, void *src)
{
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_d(var->real,*(double *)src);
	comp = dest >= 0 ? comp : -1*comp;
	b = comp <= 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_eq_const_d(int dest, void *src)
{
	(void)dest;
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_d(var->real,*(double *)src);
	b = comp == 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}

bool inst_logic_neq_const_d(int dest, void *src)
{
	(void)dest;
	Var *var = varstack_get_variable(varStack,0);
	VarValues val;
	bool b = false;
	int comp;
	comp = compare_d(var->real,*(double *)src);
	b = comp != 0 ? true : false;
	val.number = b ? 1 : 0;
	varstack_remove_nonstrings(varStack,1);
	varstack_insert_variable(varStack,TYPE_NUMBER,val);

	return true;
}
/***** *****/

/***** CONVERSION INSTRUCTIONS *****/
bool inst_conv_real(int dest,void *src)
{
	(void)src;
	Var *var = varstack_get_variable(varStack,dest);
	var->type = TYPE_REAL;
	var->real = (double)var->number;

	return true;
}

bool inst_conv_number(int dest,void *src)
{
	(void)src;
	Var *var = varstack_get_variable(varStack,dest);
	var->type = TYPE_NUMBER;
	var->number = (int)var->real;

	return true;
}
/***** *****/
