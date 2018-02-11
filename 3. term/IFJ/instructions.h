/*
 * instructions.h
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

#ifndef _IFJ_INSTRUCTIONSH_
#define _IFJ_INSTRUCTIONSH_

#include<stdio.h>
#include<stdbool.h>

#define ITRUE 	1
#define IFALSE	0



/***** I/O instructions *****/
/* if src is not NULL, output src; else output from variable dest; error on
 * outputting unitialised variable */
bool inst_output_number(int dest, void *src);
bool inst_output_real(int dest, void *src);
bool inst_output_string(int dest, void *src);

/* loads into variable dest, marks as initialised */
bool inst_input_number(int dest, void *src);
bool inst_input_real(int dest, void *src);
bool inst_input_string(int dest, void *src);
/***** *****/

/***** CONTROL instructions *****/

/* adds variable of type dest to top of stack; if src != NULL, initializes with
 * value in src (src is int*,double* or char **) */
bool inst_add_variable(int dest,void *src);
/* copies variable dest to top of stack; initialization error on copying
 * uninitialised variable */
bool inst_copy_variable(int dest,void*); 
bool inst_pop_variables(int dest,void*); //destroys dest variables

bool inst_swap(int,void*); //switches variables on top of stack

bool inst_assign(int,void *); //moves top of stack to position int and pops

bool inst_assign_const(int dest,void *src); //moves constant to dest

bool inst_run_user_fc(int dest,void*); //runs user function number dest

bool inst_run_builtin_fc(int dest,void*); //runs builtin function number dest

/* returns false if tested variable casted to int is zero; if dest == -1 then
 * takes top of stack and pops. Else takes variable dest and does not
 * pop. */
bool inst_condition(int,void*);

bool inst_jmp(int,void*); //returns false

/* returns true; pops top, pops dest variables and pushes
 * original top back to stack */
bool inst_ret(int dest,void*);

bool inst_nop(int dest,void*); //returns true

bool inst_error(int,void*); //unspecified runtime error 10
/***** *****/

/***** ARITHMETIC instructions *****/
/* pop two top instructions and push result
 * these instructions do implicit casts automatically
 * regular variants work with two top variables unless dest is not 0
 * ----------------------------------------------------
 *			+				0				-
 * 		top /- dest		top+1 /- dest	dest /- top
 * ---------------------CONST--------------------------
 *			+				-
 *		top /- dest		dest /- top
 * 
 */

bool inst_mult(int,void*);
bool inst_mult_const_i(int,void*); //const is int
bool inst_mult_const_d(int,void*); //const is double

bool inst_div(int, void *);
bool inst_div_const_i(int,void*); //const is int
bool inst_div_const_d(int,void*); //const is double

bool inst_add(int, void *);
bool inst_add_const_i(int,void*); //const is int
bool inst_add_const_d(int,void*); //const is double

bool inst_sub(int,void*);
bool inst_sub_const_i(int,void*); //const is int
bool inst_sub_const_d(int,void*); //const is double

/***** *****/

/***** LOGIC instructions *****/
/* pops two from top, pushes result
 * automatic type detection and casting; top = (int)1 if true, (int)0 if false */

/* if dest == 0 compares top+1 x top;
 * if dest > 0 compares top x dest;
 * if dest < 0 compares dest x top;
 */
bool inst_logic_gr(int,void*); //greater
bool inst_logic_le(int,void*); //lesser
bool inst_logic_greq(int,void*); //greater or equal
bool inst_logic_leeq(int,void*); //lesser or equal
bool inst_logic_eq(int,void*); //equal
bool inst_logic_neq(int,void*); //not equal

/* const variants: x cmp y; x = top if dest >= 0 */
/* integer in constant */
bool inst_logic_gr_const_i(int,void*); //greater
bool inst_logic_le_const_i(int,void*); //lesser
bool inst_logic_greq_const_i(int,void*); //greater or equal
bool inst_logic_leeq_const_i(int,void*); //lesser or equal
bool inst_logic_eq_const_i(int,void*); //equal
bool inst_logic_neq_const_i(int,void*); //not equal
/* double in constant */
bool inst_logic_gr_const_d(int,void*); //greater
bool inst_logic_le_const_d(int,void*); //lesser
bool inst_logic_greq_const_d(int,void*); //greater or equal
bool inst_logic_leeq_const_d(int,void*); //lesser or equal
bool inst_logic_eq_const_d(int,void*); //equal
bool inst_logic_neq_const_d(int,void*); //not equal
/* string in constant */
bool inst_logic_gr_const_s(int,void*); //greater
bool inst_logic_le_const_s(int,void*); //lesser
bool inst_logic_greq_const_s(int,void*); //greater or equal
bool inst_logic_leeq_const_s(int,void*); //lesser or equal
bool inst_logic_eq_const_s(int,void*); //equal
bool inst_logic_neq_const_s(int,void*); //not equal
/***** *****/

/***** CONVERSION INSTRUCTIONS *****/
/* can be called to variables of target type */
bool inst_conv_real(int dest, void*); //converts inside dest to real
bool inst_conv_number(int dest, void*); //converts inside dest to number
/***** *****/

#endif
