/*
 * sem_anal.c
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

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include "sem_anal.h"
#include "cexception.h"
#include "sem_anal_types.h"
#include "vector_string.h"
#include "vector_token.h"
#include "vector_semconst.h"
#include "ial.h"
#include "error.h"
#include "graph.h"
#include "stdarg.h"
#include "resources.h"
#include "instructions.h"
#include "vector_userfn.h"
#include "vector_exprvar.h"
#include "vector_seminstr.h"
#include "vector_unsigned.h"

#define FIRST_USER_FUNC 5

GraphTree *graphTree;
Graph *program;

typedef struct scope Scope;
typedef enum{
	SCOPE_FOR=1, //cycle_start is the third part of for, jump is the condition instruction.
	SCOPE_WHILE,
	SCOPE_DO_WHILE, //only has cycle_start
	SCOPE_IF, //jump is the condition instruction
	SCOPE_ELSE, //jump is the last instruction of the if block
	SCOPE_FUNC, //no instruction pointers but must add the error instruction
	SCOPE_OTHER,
	SCOPE_COMPOSITE_STATEMENT=0x80 //this is a flag so add any normal scope types before this
}ScopeType;

ScopeType last_ended_scope_type;

struct scope{
	Vector_String contained_variables;
	Scope *parent_scope;
	ScopeType type;
	size_t cycle_start_index;
	size_t jump_index;
};

#define NUM_TYPES 3

static Scope *current_scope=NULL;
static HTab symbol_table;
Vector_UserFn function_table;
//grows down line x86 stack
Position current_stack_top_index;
Vector_SemConst constant_store;
Vector_SemInstr inst_vect;
size_t current_rule_index;
size_t current_value_index;

#define next_rule (*vector_Rule_at(&parseTree->rules, ++current_rule_index))
#define current_rule (*vector_Rule_at(&parseTree->rules, current_rule_index))
#define next_value (*vector_Token_at(&parseTree->values, ++current_value_index))
#define current_value (*vector_Token_at(&parseTree->values, current_value_index))
#define pos_from_top(x) (x).stack_local_pos-current_stack_top_index

void add_instruction(SemInstr instr){
	vector_SemInstr_push_back(&inst_vect, instr);
}

VarType translate_semvar_type_to_type(SemVarType type){
	switch(type){
	case SEMVAR_NUMBER:
		return TYPE_NUMBER;
	case SEMVAR_REAL:
		return TYPE_REAL;
	case SEMVAR_STRING:
		return TYPE_STRING;
	case SEMVAR_FUNC:
		Throw(RET_SEM_EXP);
	}
	return TYPE_STRING;
}

VarType get_type_of_exvar(ExprVar var){
	if(var.type==EXVAR_CONST){
		return var.constant.type;
	}
	else{
		return translate_semvar_type_to_type(var.variable.type);
	}
}

bool exvar_convert_int_to_real(ExprVar *exvar){
	assert(get_type_of_exvar(*exvar)==TYPE_NUMBER);
	SemInstr instr={
		.stack_op=0,
		.const_op=SIZE_MAX,
		.top_right=false,
		.stack_adjustment=0,
		.rPtrIndex=SIZE_MAX
	};
	if(exvar->type==EXVAR_CONST){
		exvar->constant.type=TYPE_REAL;
		exvar->constant.real=exvar->constant.number;
		return false;
	}
	else{
		exvar->variable.type=SEMVAR_REAL;
		if(exvar->type==EXVAR_COMPUTED_VAR){
			instr.fPtr=inst_conv_real;
			instr.stack_op=pos_from_top(exvar->variable);
			add_instruction(instr);
			return true;
		}
		else{
			//must be a stackvar
			instr.stack_op=exvar->variable.stack_local_pos;
			exvar->type=EXVAR_COMPUTED_VAR;
			exvar->variable.stack_local_pos=--current_stack_top_index;
			instr.fPtr=inst_copy_variable;
			instr.stack_adjustment=-1;
			//copy it to the top of the stack
			add_instruction(instr);
			instr.stack_adjustment=0;
			instr.stack_op=0;
			instr.fPtr=inst_conv_real;
			//and convert
			add_instruction(instr);
			return true;
		}
	}
}

bool exvar_convert_real_to_int(ExprVar *exvar){
	assert(get_type_of_exvar(*exvar)==TYPE_REAL);
	SemInstr instr={
		.stack_op=0,
		.const_op=SIZE_MAX,
		.top_right=false,
		.stack_adjustment=0,
		.rPtrIndex=SIZE_MAX
	};
	if(exvar->type==EXVAR_CONST){
		exvar->constant.type=TYPE_NUMBER;
		exvar->constant.number=exvar->constant.real;
		return false;
	}
	else{
		exvar->variable.type=SEMVAR_REAL;
		if(exvar->type==EXVAR_COMPUTED_VAR){
			instr.stack_op=pos_from_top(exvar->variable);
			instr.fPtr=inst_conv_number;
			add_instruction(instr);
			return true;
		}
		else{
			//must be a stackvar
			instr.stack_op=exvar->variable.stack_local_pos;
			exvar->type=EXVAR_COMPUTED_VAR;
			exvar->variable.stack_local_pos=--current_stack_top_index;
			instr.fPtr=inst_copy_variable;
			instr.stack_adjustment=-1;
			//copy it to the top of the stack
			add_instruction(instr);
			instr.stack_op=0;
			instr.fPtr=inst_conv_number;
			instr.stack_adjustment=0;
			//and convert
			add_instruction(instr);
			return true;
		}
	}
}

typedef bool (*Conversion)(ExprVar*);

bool nop_conversion(ExprVar *ptr){
	return ptr->type==EXVAR_COMPUTED_VAR;
}

Conversion conversion_table[NUM_TYPES][NUM_TYPES]={
	[TYPE_REAL]={
		[TYPE_NUMBER]=exvar_convert_real_to_int,
		[TYPE_REAL]=nop_conversion
	},
	[TYPE_NUMBER]={
		[TYPE_REAL]=exvar_convert_int_to_real,
		[TYPE_NUMBER]=nop_conversion,
	},
	[TYPE_STRING]={
		[TYPE_STRING]=nop_conversion
	}
};

bool exvar_convert_to_type(VarType target_type, ExprVar *exvar){
	if(!conversion_table[get_type_of_exvar(*exvar)][target_type]){
		print_customized_message("Cannot convert types");
		Throw(RET_SEM_EXP);
		return false;
	}
	else{
		return conversion_table[get_type_of_exvar(*exvar)][target_type](exvar);
	}
}

VarType get_common_type(VarType oneType, VarType otherType){
	if(oneType==TYPE_STRING){
		return TYPE_STRING;
	}
	else if(oneType==TYPE_REAL||otherType==TYPE_REAL){
		return TYPE_REAL;
	}
	else{
		return TYPE_NUMBER;
	}
}

SemVarType translate_type_to_semvar_type(VarType type){	
	switch(type){
	case TYPE_NUMBER:
		return SEMVAR_NUMBER;
		break;
	case TYPE_REAL:
		return SEMVAR_REAL;
		break;
	case TYPE_STRING:
		return SEMVAR_STRING;
		break;
	}
}

SemInstr create_instruction_push_const(SemConst constant){
	vector_SemConst_push_back(&constant_store, constant);
	return (SemInstr){
		.rPtrIndex=SIZE_MAX,
		.const_op=vector_SemConst_count(&constant_store)-1,
		.stack_op=constant.type,
		.stack_adjustment=-1,
		.top_right=false,
		.fPtr=inst_add_variable
	};
}

void add_instruction_push_const(ExprVar *exvar){
	assert(exvar->type==EXVAR_CONST);
	SemInstr ninst=create_instruction_push_const(exvar->constant);
	exvar->type=EXVAR_COMPUTED_VAR;
	exvar->variable.type=translate_type_to_semvar_type(exvar->constant.type);
	exvar->variable.stack_local_pos=--current_stack_top_index;
	add_instruction(ninst);
}

SemInstr create_instruction_copy_stackvar(SemVar stackvar){
	return (SemInstr){
		.rPtrIndex=SIZE_MAX,
		.const_op=SIZE_MAX,
		.stack_op=stackvar.stack_local_pos,
		.stack_adjustment=-1,
		.top_right=false,
		.fPtr=inst_copy_variable
	};
}

void add_instruction_copy_stackvar(ExprVar *stackvar){
	assert(stackvar->type==EXVAR_STACKVAR);
	SemInstr instr=create_instruction_copy_stackvar(stackvar->variable);
	add_instruction(instr);
	stackvar->type=EXVAR_COMPUTED_VAR;
	stackvar->variable.stack_local_pos=--current_stack_top_index;
}

void add_instruction_func_call(Token func_name, unsigned given_param_count, Vector_ExprVar *expr_stack){
	SemInstr instr={
		.rPtrIndex=SIZE_MAX,
		.const_op=SIZE_MAX,
		.top_right=false
	};
	SemVar *func_loc=htab_search(&symbol_table, func_name.IDName);
	if(!func_loc){
		error_message("Undefined function %s.", func_name.line, func_name.IDName);
		Throw(RET_SEM_DEF);
	}
	else if(func_loc->type!=SEMVAR_FUNC){
		error_message("\'%s' is not a function.", func_name.line, func_name.IDName);
		Throw(RET_SEM_EXP);
	}
	UserFn func=*vector_UserFn_at(&function_table, func_loc->stack_local_pos);
	size_t func_param_count=vector_FuncParam_count(&func.params);
	if(func_param_count!=given_param_count){
		error_message("Wrong number of parameters in call of function \'%s\'",
				func_name.line, func_name.IDName);
		Throw(RET_SEM_EXP);
	}
	//make sure the parameters have the correct types.
	for(size_t i=func_param_count; i>0; i--){
		FuncParam param=*vector_FuncParam_at(&func.params, i-1);
		ExprVar tmp=vector_ExprVar_pop_back(expr_stack);
		assert(tmp.type==EXVAR_COMPUTED_VAR);
		exvar_convert_to_type(translate_semvar_type_to_type(param.type), &tmp);
	}
	instr.stack_op=func_loc->stack_local_pos;
	if(func_loc->stack_local_pos<FIRST_USER_FUNC){
		//it's a builtin call
		instr.fPtr=inst_run_builtin_fc;
	}
	else{
		//builtins don't exist as functions during execution, so actual function indexes must be shifted.
		instr.stack_op-=FIRST_USER_FUNC;
		instr.fPtr=inst_run_user_fc;
	}
	instr.stack_adjustment=((Position)func_param_count-1);
	add_instruction(instr);
	current_stack_top_index+=instr.stack_adjustment;
	ExprVar retvar={
		.type=EXVAR_COMPUTED_VAR,
		.variable=(SemVar){
			.type=func.return_type,
			.stack_local_pos=current_stack_top_index
		}
	};
	vector_ExprVar_push_back(expr_stack, retvar);
}

void create_new_scope(ScopeType type, size_t jump, size_t cycle_start){
	Scope *nscope=malloc(sizeof(Scope));
	if(!nscope){
		Throw(RET_INTERN);
	}
	nscope->parent_scope=current_scope;
	vector_String_init(&nscope->contained_variables);
	nscope->type=type;
	nscope->jump_index=jump;
	nscope->cycle_start_index=cycle_start;
	current_scope=nscope;
}

int analyze_condition(ParseTree *parseTree);

int end_scope(ParseTree *parseTree){
	int retval=RET_EOK;
	last_ended_scope_type=current_scope->type&~SCOPE_COMPOSITE_STATEMENT;
	SemInstr instr;
	instr.fPtr=inst_pop_variables;
	instr.rPtrIndex=SIZE_MAX;
	instr.const_op=SIZE_MAX;
	instr.stack_op=vector_String_count(&current_scope->contained_variables);
	instr.stack_adjustment=instr.stack_op;
	current_stack_top_index+=instr.stack_op;
	instr.top_right=false;
	//don't create an empty pop instruction
	if(instr.stack_op>0){
		add_instruction(instr);
	}
	for(Position i=0; i<instr.stack_op; i++){
		String varName=vector_String_pop_back(&current_scope->contained_variables);
		htab_remove(&symbol_table, varName);
	}
	vector_String_destroy(&current_scope->contained_variables);
	instr.stack_op=0;
	instr.stack_adjustment=0;
	switch(current_scope->type&~SCOPE_COMPOSITE_STATEMENT){
	case SCOPE_IF:
		//generate a jump. By default it will just jump to the next instruction
		//if an else block happens, it will set it to point to the end of itself.
		instr.fPtr=inst_jmp;
		instr.rPtrIndex=vector_SemInstr_count(&inst_vect)+1;
		add_instruction(instr);
		//set the jump to next instruction
		vector_SemInstr_at(&inst_vect, current_scope->jump_index)->rPtrIndex=vector_SemInstr_count(&inst_vect);
		break;
	case SCOPE_ELSE:
		//make sure that the last instruction of the if actually skips the else block
		vector_SemInstr_at(&inst_vect, current_scope->jump_index)->rPtrIndex=vector_SemInstr_count(&inst_vect);
		break;
	case SCOPE_FOR:
		//do not pop the iteration variable
		{
			SemInstr *pop_inst=vector_SemInstr_back(&inst_vect);
			if(pop_inst->stack_op==1){
				vector_SemInstr_pop_back(&inst_vect);
			}
			else{
				pop_inst->stack_op--;
				pop_inst->stack_adjustment--;
			}
		}
		//jump to the cycle start
		instr.fPtr=inst_jmp;
		instr.rPtrIndex=current_scope->cycle_start_index;
		add_instruction(instr);
		//set condition to jump to cycle end
		vector_SemInstr_at(&inst_vect, current_scope->jump_index)->rPtrIndex=vector_SemInstr_count(&inst_vect);
		//now pop the iteration variable
		instr.rPtrIndex=SIZE_MAX;
		instr.fPtr=inst_pop_variables;
		instr.stack_adjustment=1;
		instr.stack_op=1;
		add_instruction(instr);
		break;
	case SCOPE_WHILE:
		instr.fPtr=inst_jmp;
		instr.rPtrIndex=current_scope->cycle_start_index;
		add_instruction(instr);
		vector_SemInstr_at(&inst_vect, current_scope->jump_index)->rPtrIndex=vector_SemInstr_count(&inst_vect);
		break;
	case SCOPE_DO_WHILE:
		current_rule_index++;
		retval=analyze_condition(parseTree);
		//TODO: reverse
		vector_SemInstr_back(&inst_vect)->rPtrIndex=vector_SemInstr_count(&inst_vect)+1;
		instr.fPtr=inst_jmp;
		instr.rPtrIndex=current_scope->cycle_start_index;
		add_instruction(instr);
		break;
	case SCOPE_FUNC:
		//if function ends, the last instruction must be an error so that it throws when there is no return.
		instr.stack_op=RET_UNINIT;
		instr.fPtr=inst_error;
		add_instruction(instr);
		break;
	case SCOPE_OTHER:
		//do nothing special
		break;
	}
	Scope *to_free=current_scope;
	current_scope=current_scope->parent_scope;
	free(to_free);
	return retval;
}

/**
 * @brief Creates a new semantic variable. DOES NOT GENERATE ANY INSTRUCTIONS.
 * @param name The name of the variable
 * @param type The type of the variable
 * @return RET_EOK on success, RET_SEM_DEF if a variable with the given name cannot be created in the current scope.
 */
int create_new_semantic_variable(SemVarType type, String name){
	SemVar *tmp=htab_search(&symbol_table, name);
	if(tmp){
		//variables must not share name with a function
		if(tmp->type==SEMVAR_FUNC){
			print_customized_message("Tried to redefine function \'%s\'.", name);
			return RET_SEM_DEF;
		}
		size_t current_scope_varcount=vector_String_count(&current_scope->contained_variables);
		//they also must have a name distinct from other variables in the same scope.
		if(pos_from_top(*tmp) <= (Position)current_scope_varcount){
			print_customized_message("Tried to redefine variable \'%s\'.", name);
			return RET_SEM_DEF;
		}
	}
	vector_String_push_back(&current_scope->contained_variables, name);
	//named variables cannot be created inside of an expression
	SemVar nvar={.stack_local_pos=current_stack_top_index, .type=type};
	htab_insert(&symbol_table, name, nvar);
	return RET_EOK;
}

//returns the pointer to the structure containing the information about the function identified by name
UserFn *create_new_function(String name){
	SemVar *tmp=htab_search(&symbol_table, name);
	//if there is another symbol with this name
	if(tmp){
		//the symbol must be a function name, no global variables
		return vector_UserFn_at(&function_table, tmp->stack_local_pos);
	}
	//return type of FUNC indicates that this is a new function.
	UserFn nfunc={.return_type=SEMVAR_FUNC, .first_inst_index=SIZE_MAX};
	vector_FuncParam_init(&nfunc.params);
	size_t current_func_count=vector_UserFn_count(&function_table);
	htab_insert(&symbol_table, name, (SemVar){.type=SEMVAR_FUNC, .stack_local_pos=current_func_count});
	vector_UserFn_push_back(&function_table, nfunc);
	return vector_UserFn_at(&function_table, current_func_count);
}

void destroy_last_declared_function(String name){
	htab_remove(&symbol_table, name);
	vector_UserFn_pop_back(&function_table);
}

int func_add_param(UserFn *func, SemVarType type, String name){
	size_t prevNumParams=vector_FuncParam_count(&func->params);
	for(unsigned i=0; i<prevNumParams; i++){
		if(!strcmp(name, vector_FuncParam_at(&func->params, i)->name)){
			print_customized_message("redefinition of parameter \'%s\'");
			return RET_SEM_DEF;
		}
	}
	if(htab_search(&symbol_table, name)){
		print_customized_message("redefinition of function \'%s\'");
		return RET_SEM_DEF;
	}
	FuncParam nparam={.name=name, .type=type};
	vector_FuncParam_push_back(&func->params, nparam);
	return RET_EOK;
}

typedef enum {
	SEM_ERR_OK,
	SEM_ERR_TOO_MANY_PARAMS,
	SEM_ERR_WRONG_PARAM_NAME,
}SemErrors;

/**
 * @brief Checks if the parameter has the given values.
 * @param func The function agaisnt whose parameters to check
 * @param type The type of the parameter
 * @param name The name of the parameter
 * @param param_index The index of the parameter
 * @return An enum indicating possible errors
 */
SemErrors func_check_param(UserFn *func, SemVarType type, String name, size_t param_index){
	//if the function doesn't have that many params, fail
	if(param_index>=vector_FuncParam_count(&func->params)){
		return SEM_ERR_TOO_MANY_PARAMS;
	}
	FuncParam *correct_param=vector_FuncParam_at(&func->params, param_index);
	if(correct_param->type!=type||strcmp(name, correct_param->name)){
		return SEM_ERR_WRONG_PARAM_NAME;
	}
	else{
		return SEM_ERR_OK;
	}
}

SemVarType translate_rule_to_semvar_type(Rule rule){
	switch(rule){
	case RULE_TYPE__DOUBLE:
		return SEMVAR_REAL;
	case RULE_TYPE__INT:
		return SEMVAR_NUMBER;
	case RULE_TYPE__STRING:
		return SEMVAR_STRING;
	default:
		print_customized_message("Expected type rule in semantic analysis but got something else.");
		Throw(RET_INTERN);
		return SEMVAR_FUNC;
	}
}

void handle_param_mismatch(SemErrors err, Token paramName, String funcName){
	if(err==SEM_ERR_TOO_MANY_PARAMS){
		error_message("Function %s declared with a different number of parameters from previous declarations.", paramName.line, funcName);
	}
	else if(err==SEM_ERR_WRONG_PARAM_NAME){
		error_message("Parameter %s of function %s declared with a different name or type from previous declarations.",
			paramName.line, paramName.IDName, funcName);
	}
}

UserFn* analyze_func_decl(ParseTree *parseTree){
	current_rule_index++;
	assert(current_rule==RULE_FUNC_DECL);
	SemVarType retType=translate_rule_to_semvar_type(next_rule);
	UserFn *fn=create_new_function(next_value.IDName);
	String func_name=current_value.IDName;
	if(fn->return_type==SEMVAR_FUNC){
		//this function has never been declared.
		fn->return_type=retType;
		if(next_rule==RULE_FUNC_DECL_ARG_LIST__EPSILON){
			//if there are no arguments, just end
			return fn;
		}
		assert(current_rule=RULE_FUNC_DECL_ARG_LIST__DECL_ARG_CONT);
		do{
			if(func_add_param(fn, translate_rule_to_semvar_type(next_rule), next_value.IDName)!=RET_EOK){
				error_message("Redefinition of \'%s\' in declaration of function \'%s\'.", current_value.line, current_value.IDName, func_name);
				//this declaration is invalid, so make sure it doesn't stay somewhere
				destroy_last_declared_function(func_name);
				return NULL;
			}
		}while(next_rule==RULE_FUNC_DECL_ARG_LIST_CONT__COMMA_DECL_ARG_CONT);
		assert(current_rule==RULE_FUNC_DECL_ARG_LIST_CONT__EPSILON);
		//all parameters were OK
		return fn;
	}
	else{
		//check the return type
		if(fn->return_type!=retType){
			error_message("Function %s declared with a different return type from previous declarations.", current_value.line, current_value.IDName);
			return NULL;
		}
		//now check that the parameters are correct
		if(next_rule==RULE_FUNC_DECL_ARG_LIST__EPSILON){
			if(vector_FuncParam_count(&fn->params)!=0){
				error_message("Function %s declared with a different number of parameters from previous declarations.", current_value.line, current_value.IDName);
				return NULL;
			}
			else{
				return fn;
			}
		}
		SemErrors param_retval=func_check_param(fn, translate_rule_to_semvar_type(next_rule), next_value.IDName, 0);
		if(param_retval!=SEM_ERR_OK){
			handle_param_mismatch(param_retval, current_value, func_name);
			return NULL;
		}
		size_t i;
		for(i=1; next_rule==RULE_FUNC_DECL_ARG_LIST_CONT__COMMA_DECL_ARG_CONT; i++){
			param_retval=func_check_param(fn, translate_rule_to_semvar_type(next_rule), next_value.IDName, i);
			if(param_retval!=SEM_ERR_OK){
				handle_param_mismatch(param_retval, current_value, func_name);
				return NULL;
			}
		}
		if(vector_FuncParam_count(&fn->params)!=i){
			error_message("Function %s declared with a different number of parameters from previous declarations.", current_value.line, current_value.IDName);
			return NULL;
		}
		else{
			return fn;
		}
	}
	//not going to be reached
	return NULL;
}

typedef enum{
	OPERAND_STACK,
	OPERAND_CONST,
	OPERAND_COMPUTED,
	OPERAND_EXECUTE_IMMEDIATELY
}OperandType;

SemVarType get_result_type(SemVarType common_type, Rule operation){
	switch(operation){
	case RULE_EXPR__ADD:
	case RULE_EXPR__SUB:
	case RULE_EXPR__DIV:
	case RULE_EXPR__MULT:
		return common_type;
	case RULE_EXPR__EQ:
	case RULE_EXPR__GREATER:
	case RULE_EXPR__GREATEREQ:
	case RULE_EXPR__LESS:
	case RULE_EXPR__LESSEQ:
	case RULE_EXPR__NEQ:
		return SEMVAR_NUMBER;
	default:
		print_customized_message("unexpected nonarithmetic operation");
		Throw(RET_INTERN);
		return SEMVAR_FUNC;
	}
}

/**
 * @brief Prepares for an arithmetic operation. Sets dest to the necessary value and optionally copies a stack variable to the top of the stack.
 * @param exvar1 The first operand
 * @param exvar2 The second operand
 * @param dest dest of the arithmetic/compare instruction
 * @return The type of operand to operate witht he top of the stack.
 */
OperandType prepare_op(ExprVar *exvar1, ExprVar *exvar2, SemInstr *instr){
	//if top is on the right side of op, *dest must be negative
	VarType common_type=get_common_type(get_type_of_exvar(*exvar1), get_type_of_exvar(*exvar2));
	size_t current_instr_pos=vector_SemInstr_count(&inst_vect);
	exvar_convert_to_type(common_type, exvar1);
	exvar_convert_to_type(common_type, exvar2);
	instr->top_right=true;
	instr->stack_adjustment=0;
	switch(exvar1->type){
	case EXVAR_CONST:
		switch(exvar2->type){
		case EXVAR_CONST:
			return OPERAND_EXECUTE_IMMEDIATELY;
			break;
		case EXVAR_STACKVAR:
			add_instruction_copy_stackvar(exvar2);
		case EXVAR_COMPUTED_VAR:
			//const op top
			instr->stack_op=1;
			return OPERAND_CONST;
		}
	case EXVAR_STACKVAR:
		switch(exvar2->type){
		case EXVAR_CONST:
			add_instruction_copy_stackvar(exvar1);
			//top op const
			instr->stack_op=1;
			instr->top_right=false;
			return OPERAND_CONST;
		case EXVAR_STACKVAR:
			add_instruction_copy_stackvar(exvar2);
		case EXVAR_COMPUTED_VAR:
			//stack op top
			instr->stack_op=exvar1->variable.stack_local_pos;
			return OPERAND_STACK;
		}
	case EXVAR_COMPUTED_VAR:
		instr->top_right=false;
		switch(exvar2->type){
		case EXVAR_CONST:
			//top op const
			instr->stack_op=1;
			return OPERAND_CONST;
		case EXVAR_STACKVAR:
			//top op stack
			instr->stack_op=exvar2->variable.stack_local_pos;
			return OPERAND_STACK;
		case EXVAR_COMPUTED_VAR:
			//if the varaible order got swapped in the conversion
			if(exvar1->variable.stack_local_pos<exvar2->variable.stack_local_pos){
				size_t inst_count=vector_SemInstr_count(&inst_vect);
				size_t ninst_size=(inst_count-current_instr_pos)*sizeof(SemInstr);
				SemInstr *temp_store=malloc(ninst_size);
				if(temp_store==NULL){
					Throw(RET_INTERN);
				}
				SemInstr *ninst_start=vector_SemInstr_at(&inst_vect, current_instr_pos);
				//move the conversion instructions to a temporary location
				memcpy(temp_store, ninst_start, ninst_size);
				SemInstr *orig_loc=vector_SemInstr_at(&inst_vect, exvar1->current_instr_pos);
				//make space for the conversion instructions
				memmove(((char*)orig_loc)+ninst_size, orig_loc, (ninst_start-orig_loc)*sizeof(SemInstr));
				//copy the conversion instructions to the place they belong
				memcpy(orig_loc, temp_store, ninst_size);
				free(temp_store);
			}
			instr->stack_op=0;
			instr->stack_adjustment=1;
			return OPERAND_COMPUTED;
		}
	}
}

#define set_real_retval(x) real_retval=real_retval==RET_EOK?x:real_retval;

void evaluate_constexpr(Rule operation, ExprVar exvar1, ExprVar exvar2, Vector_ExprVar *expr_stack){
	switch(operation){
	case RULE_EXPR__ADD:
		if(exvar1.constant.type==TYPE_NUMBER){
			exvar1.constant.number=exvar1.constant.number+exvar2.constant.number;
		}
		else{
			exvar1.constant.real=exvar1.constant.real+exvar2.constant.real;
		}
		break;
	case RULE_EXPR__SUB:
		if(exvar1.constant.type==TYPE_NUMBER){
			exvar1.constant.number=exvar1.constant.number-exvar2.constant.number;
		}
		else{
			exvar1.constant.real=exvar1.constant.real-exvar2.constant.real;
		}
		break;
	case RULE_EXPR__DIV:
		if(exvar1.constant.type==TYPE_NUMBER){
			if(!exvar2.constant.number){
				SemInstr instr;
				instr.rPtrIndex=SIZE_MAX;
				instr.stack_adjustment=0;
				instr.const_op=SIZE_MAX;
				instr.stack_op=RET_DIV;
				instr.top_right=false;
				instr.fPtr=inst_error;
				add_instruction(instr);
			}
			else{
				exvar1.constant.number=exvar1.constant.number/exvar2.constant.number;
			}
		}
		else{
			if(exvar2.constant.real==0){
				SemInstr instr;
				instr.rPtrIndex=SIZE_MAX;
				instr.stack_adjustment=0;
				instr.const_op=SIZE_MAX;
				instr.stack_op=RET_DIV;
				instr.top_right=false;
				instr.fPtr=inst_error;
				add_instruction(instr);
			}
			else{
				exvar1.constant.real=exvar1.constant.real/exvar2.constant.real;
			}
		}
		break;
	case RULE_EXPR__MULT:
		if(exvar1.constant.type==TYPE_NUMBER){
			exvar1.constant.number=exvar1.constant.number*exvar2.constant.number;
		}
		else{
			exvar1.constant.real=exvar1.constant.real*exvar2.constant.real;
		}
		break;
	case RULE_EXPR__EQ:
		if(exvar1.constant.type==TYPE_NUMBER){
			exvar1.constant.number = exvar1.constant.number==exvar2.constant.number;
		}
		else if(exvar1.constant.type==TYPE_REAL){
			exvar1.constant.number = exvar1.constant.real==exvar2.constant.real;
		}
		else{
			exvar1.constant.number = !strcmp(exvar1.constant.string->data, exvar2.constant.string->data);
		}
		exvar1.constant.type=TYPE_NUMBER;
		break;
	case RULE_EXPR__GREATER:
		if(exvar1.constant.type==TYPE_NUMBER){
			exvar1.constant.number = exvar1.constant.number>exvar2.constant.number;
		}
		else if(exvar1.constant.type==TYPE_REAL){
			exvar1.constant.number = exvar1.constant.real>exvar2.constant.real;
		}
		else{
			exvar1.constant.number = strcmp(exvar1.constant.string->data, exvar2.constant.string->data)>0;
		}
		exvar1.constant.type=TYPE_NUMBER;
		break;
	case RULE_EXPR__GREATEREQ:
		if(exvar1.constant.type==TYPE_NUMBER){
			exvar1.constant.number = exvar1.constant.number>=exvar2.constant.number;
		}
		else if(exvar1.constant.type==TYPE_REAL){
			exvar1.constant.number = exvar1.constant.real>=exvar2.constant.real;
		}
		else{
			exvar1.constant.number = strcmp(exvar1.constant.string->data, exvar2.constant.string->data)>=0;
		}
		exvar1.constant.type=TYPE_NUMBER;
		break;
	case RULE_EXPR__LESS:
		if(exvar1.constant.type==TYPE_NUMBER){
			exvar1.constant.number = exvar1.constant.number<exvar2.constant.number;
		}
		else if(exvar1.constant.type==TYPE_REAL){
			exvar1.constant.number = exvar1.constant.real<exvar2.constant.real;
		}
		else{
			exvar1.constant.number = strcmp(exvar1.constant.string->data, exvar2.constant.string->data)<0;
		}
		exvar1.constant.type=TYPE_NUMBER;
		break;
	case RULE_EXPR__LESSEQ:
		if(exvar1.constant.type==TYPE_NUMBER){
			exvar1.constant.number = exvar1.constant.number<=exvar2.constant.number;
		}
		else if(exvar1.constant.type==TYPE_REAL){
			exvar1.constant.number = exvar1.constant.real<=exvar2.constant.real;
		}
		else{
			exvar1.constant.number = strcmp(exvar1.constant.string->data, exvar2.constant.string->data)<=0;
		}
		exvar1.constant.type=TYPE_NUMBER;
		break;
	case RULE_EXPR__NEQ:
		if(exvar1.constant.type==TYPE_NUMBER){
			exvar1.constant.number = exvar1.constant.number!=exvar2.constant.number;
		}
		else if(exvar1.constant.type==TYPE_REAL){
			exvar1.constant.number = exvar1.constant.real!=exvar2.constant.real;
		}
		else{
			exvar1.constant.number = !!strcmp(exvar1.constant.string->data, exvar2.constant.string->data);
		}
		exvar1.constant.type=TYPE_NUMBER;
		break;
	default:
		fprintf(stderr, "unexpected rule in constant operation\n");
		Throw(RET_INTERN);
	}
	exvar1.current_instr_pos=vector_SemInstr_count(&inst_vect);
	vector_ExprVar_push_back(expr_stack, exvar1);
}

void add_instruction_operate_with_int_const(SemInstr instr, Rule op){
	switch(op){
	case RULE_EXPR__ADD:
		instr.fPtr=inst_add_const_i;
		break;
	case RULE_EXPR__SUB:
		instr.fPtr=inst_sub_const_i;
		break;
	case RULE_EXPR__DIV:
		instr.fPtr=inst_div_const_i;
		break;
	case RULE_EXPR__MULT:
		instr.fPtr=inst_mult_const_i;
		break;
	case RULE_EXPR__EQ:
		instr.fPtr=inst_logic_eq_const_i;
		break;
	case RULE_EXPR__GREATER:
		instr.fPtr=inst_logic_gr_const_i;
		break;
	case RULE_EXPR__GREATEREQ:
		instr.fPtr=inst_logic_greq_const_i;
		break;
	case RULE_EXPR__LESS:
		instr.fPtr=inst_logic_le_const_i;
		break;
	case RULE_EXPR__LESSEQ:
		instr.fPtr=inst_logic_leeq_const_i;
		break;
	case RULE_EXPR__NEQ:
		instr.fPtr=inst_logic_neq_const_i;
		break;
	default:
		fprintf(stderr, "unexpected rule in int const instr gen.\n");
		Throw(RET_INTERN);
	}
	add_instruction(instr);
}

void add_instruction_operate_with_real_const(SemInstr instr, Rule op){
	switch(op){
	case RULE_EXPR__ADD:
		instr.fPtr=inst_add_const_d;
		break;
	case RULE_EXPR__SUB:
		instr.fPtr=inst_sub_const_d;
		break;
	case RULE_EXPR__DIV:
		instr.fPtr=inst_div_const_d;
		break;
	case RULE_EXPR__MULT:
		instr.fPtr=inst_mult_const_d;
		break;
	case RULE_EXPR__EQ:
		instr.fPtr=inst_logic_eq_const_d;
		break;
	case RULE_EXPR__GREATER:
		instr.fPtr=inst_logic_gr_const_d;
		break;
	case RULE_EXPR__GREATEREQ:
		instr.fPtr=inst_logic_greq_const_d;
		break;
	case RULE_EXPR__LESS:
		instr.fPtr=inst_logic_le_const_d;
		break;
	case RULE_EXPR__LESSEQ:
		instr.fPtr=inst_logic_leeq_const_d;
		break;
	case RULE_EXPR__NEQ:
		instr.fPtr=inst_logic_neq_const_d;
		break;
	default:
		fprintf(stderr, "unexpected rule in real const instr gen.\n");
		Throw(RET_INTERN);
	}
	add_instruction(instr);
}

void add_instruction_operate_with_string_const(SemInstr instr, Rule op){
	switch(op){
	case RULE_EXPR__EQ:
		instr.fPtr=inst_logic_eq_const_s;
		break;
	case RULE_EXPR__GREATER:
		instr.fPtr=inst_logic_gr_const_s;
		break;
	case RULE_EXPR__GREATEREQ:
		instr.fPtr=inst_logic_greq_const_s;
		break;
	case RULE_EXPR__LESS:
		instr.fPtr=inst_logic_le_const_s;
		break;
	case RULE_EXPR__LESSEQ:
		instr.fPtr=inst_logic_leeq_const_s;
		break;
	case RULE_EXPR__NEQ:
		instr.fPtr=inst_logic_neq_const_s;
		break;
	default:
		fprintf(stderr, "unexpected rule in string const instr gen.\n");
		Throw(RET_INTERN);
	}
	add_instruction(instr);
}

/**
 * @brief Analyzes the expression as a string in postfix notation.
 * It will "execute" the string of rules, but will only generate instructions when necessary.
 * @param parseTree The rules and values to analyze
 * @param result The result will be stored at the location pointed by this parameter.
 * @return TODO: decide what this actually returns.
 */
int analyze_expression(ParseTree *parseTree, ExprVar *result){
	int retval;
	int real_retval=RET_EOK;
	Vector_ExprVar expr_stack;
	retval=vector_ExprVar_init(&expr_stack);
	if(retval!=RET_EOK){
		return RET_INTERN;
	}
	Vector_unsigned param_counts;
	retval=vector_unsigned_init(&param_counts);
	if(retval!=RET_EOK){
		vector_ExprVar_destroy(&expr_stack);
		return RET_INTERN;
	}
	ExprVar exvar1, exvar2;
	SemVar *foundVar;
	SemInstr instr;
	Try{
		OperandType operandType;
		//while we are in an expression
		while(next_rule!=RULE_EXPR_END__EPSILON){
			assert(current_rule>RULES_EXPRESSIONS_START);
			instr.stack_adjustment=0;
			instr.stack_op=0;
			instr.const_op=SIZE_MAX;
			instr.top_right=false;
			instr.rPtrIndex=SIZE_MAX;
			bool handling_operator=false;
			switch(current_rule){
			case RULE_EXPR__LEX_ID:
				exvar1.type=EXVAR_STACKVAR;
				exvar1.current_instr_pos=vector_SemInstr_count(&inst_vect);
				foundVar=htab_search(&symbol_table, next_value.IDName);
				if(!foundVar){
					error_message("undefined variable \'%s\'", current_value.line, current_value.IDName);
					Throw(RET_SEM_DEF);
				}
				exvar1.variable=*foundVar;
				vector_ExprVar_push_back(&expr_stack, exvar1);
				break;
			case RULE_EXPR__LEX_NUMBER:
				exvar1.current_instr_pos=vector_SemInstr_count(&inst_vect);
				exvar1.type=EXVAR_CONST;
				exvar1.constant.type=TYPE_NUMBER;
				exvar1.constant.number=next_value.number;
				vector_ExprVar_push_back(&expr_stack, exvar1);
				break;
			case RULE_EXPR__LEX_REAL_NUMBER:
				exvar1.current_instr_pos=vector_SemInstr_count(&inst_vect);
				exvar1.type=EXVAR_CONST;
				exvar1.constant.type=TYPE_REAL;
				exvar1.constant.real=next_value.real;
				vector_ExprVar_push_back(&expr_stack, exvar1);
				break;
			case RULE_EXPR__LEX_STRING_LITERAL:
				exvar1.current_instr_pos=vector_SemInstr_count(&inst_vect);
				exvar1.type=EXVAR_CONST;
				exvar1.constant.type=TYPE_STRING;
				exvar1.constant.string=malloc(sizeof(VarString)+strlen(next_value.string)+1);
				if(!exvar1.constant.string){
					Throw(RET_INTERN);
				}
				strcpy(exvar1.constant.string->data, current_value.string);
				exvar1.constant.string->length=strlen(exvar1.constant.string->data);
				exvar1.constant.string->references=1;
				vector_ExprVar_push_back(&expr_stack, exvar1);
				break;
			case RULE_EXPR__ADD:
			case RULE_EXPR__SUB:
			case RULE_EXPR__DIV:
			case RULE_EXPR__MULT:
				exvar2=vector_ExprVar_pop_back(&expr_stack);
				exvar1=vector_ExprVar_pop_back(&expr_stack);
				operandType=prepare_op(&exvar1, &exvar2, &instr);
				handling_operator=true;
				break;
			case RULE_EXPR__EQ:
			case RULE_EXPR__GREATER:
			case RULE_EXPR__GREATEREQ:
			case RULE_EXPR__LESS:
			case RULE_EXPR__LESSEQ:
			case RULE_EXPR__NEQ:
				exvar2=vector_ExprVar_pop_back(&expr_stack);
				exvar1=vector_ExprVar_pop_back(&expr_stack);
				operandType=prepare_op(&exvar1, &exvar2, &instr);
				handling_operator=true;
				break;
			case RULE_EXPR__PARENTHESES_EXPR:
				break;
			case RULE_EXPR__FUNC_PARAM_LIST:
				//If the parameters are in a [func_call_arg_list], then the variables were (if necessary) pushed
				//when the param list was being created, so this is essentially identical to a no-param call.
				add_instruction_func_call(next_value, vector_unsigned_pop_back(&param_counts), &expr_stack);
				break;
			case RULE_EXPR__FUNC_NO_PARAM:
				add_instruction_func_call(next_value, 0, &expr_stack);
				break;
			case RULE_EXPR__FUNC_ONE_PARAM:
				exvar1=vector_ExprVar_pop_back(&expr_stack);
				if(exvar1.type==EXVAR_CONST){
					add_instruction_push_const(&exvar1);
				}
				else if(exvar1.type==EXVAR_STACKVAR){
					add_instruction_copy_stackvar(&exvar1);
				}
				//it's always a compvar now, push it back on the stack
				vector_ExprVar_push_back(&expr_stack, exvar1);
				add_instruction_func_call(next_value, 1, &expr_stack);
				break;
			case RULE_FUNC_CALL_ARG_LIST__EXPR_EXPR:
				exvar2=vector_ExprVar_pop_back(&expr_stack);
				exvar1=vector_ExprVar_pop_back(&expr_stack);
				vector_unsigned_push_back(&param_counts, 2);
				if(exvar1.type!=EXVAR_COMPUTED_VAR){
					//if it's not a computed value, no instruction was generated to push it to the execution varstack
					//Therefore we must hack a little to go back in time and make sure it is pushed before the expression in the second parameter is evaluated.
					if(exvar2.type!=EXVAR_COMPUTED_VAR){
						//unless, of course, the other parameter is also a constant or a stackvar and therefore wasn't pushed.
						//in that case, we can just push them in order, no time travel necessary.
						if(exvar1.type==EXVAR_CONST){
							add_instruction_push_const(&exvar1);
						}
						else{
							add_instruction_copy_stackvar(&exvar1);
						}
						if(exvar2.type==EXVAR_CONST){
							add_instruction_push_const(&exvar2);
						}
						else{
							//it's a stackvar
							add_instruction_copy_stackvar(&exvar2);
						}
						vector_ExprVar_push_back(&expr_stack, exvar1);
						vector_ExprVar_push_back(&expr_stack, exvar2);
					}
					else{
						//The new instruction must be inserted at where it would be when the constant was encountered.
						if(exvar1.type==EXVAR_CONST){
							instr=create_instruction_push_const(exvar1.constant);
							SemVarType nvarType=translate_type_to_semvar_type(exvar1.constant.type);
							exvar1.variable.type=nvarType;
						}
						else{
							instr=create_instruction_copy_stackvar(exvar1.variable);
						}
						size_t insert_at=exvar1.current_instr_pos;
						exvar1.type=EXVAR_COMPUTED_VAR;
						exvar1.variable.stack_local_pos=--current_stack_top_index;
						//the stack variable positions are relative, the computed variable positions are not
						//and there are no jumps inside expressions.
						vector_SemInstr_insert_at(&inst_vect, insert_at, instr);
						vector_ExprVar_push_back(&expr_stack, exvar1);
						vector_ExprVar_push_back(&expr_stack, exvar2);
					}
				}
				else{
					//exvar1 is computed so just push it back
					vector_ExprVar_push_back(&expr_stack, exvar1);
					switch(exvar2.type){
					case EXVAR_CONST:
						add_instruction_push_const(&exvar2);
						break;
					case EXVAR_STACKVAR:
						add_instruction_copy_stackvar(&exvar2);
						break;
					case EXVAR_COMPUTED_VAR:
						break;
					}
					//now push exvar2 back where it was
					vector_ExprVar_push_back(&expr_stack, exvar2);
				}
				break;
			case RULE_FUNC_CALL_ARG_LIST__FUNC_CALL_ARG_LIST_EXPR:
				(*vector_unsigned_back(&param_counts))++;
				//the previous params are already on the stack
				//if the next param is a compvar, it's already on the stack, so no action necessary
				if(vector_ExprVar_back(&expr_stack)->type!=EXVAR_COMPUTED_VAR){
					exvar1=vector_ExprVar_pop_back(&expr_stack);
					if(exvar1.type==EXVAR_CONST){
						add_instruction_push_const(&exvar1);
					}
					else{
						add_instruction_copy_stackvar(&exvar1);
					}
					vector_ExprVar_push_back(&expr_stack, exvar1);
				}
				break;
			default:
				fprintf(stderr, "Unexpected rule in expression\n");
				Throw(RET_INTERN);
			}
			if(handling_operator){
				//exvar 1 and 2 contain the operands,
				ExprVar operand, top;
				if(instr.top_right){
					operand=exvar1;
					top=exvar2;
				}
				else{
					operand=exvar2;
					top=exvar1;
				}
				switch(operandType){
				case OPERAND_EXECUTE_IMMEDIATELY:
					//the function adds the resulting constant to the stack
					evaluate_constexpr(current_rule, exvar1, exvar2, &expr_stack);
					//the constants are lost, so free them
					if(exvar1.constant.type==TYPE_STRING){
						free(exvar1.constant.string);
						free(exvar2.constant.string);
						//the strings themselves will be freed at semantic analysis cleanup.
					}
					break;
				case OPERAND_CONST:
					instr.const_op=vector_SemConst_count(&constant_store);
					//top already has the correct stack local pos
					top.variable.type=get_result_type(top.variable.type, current_rule);
					vector_SemConst_push_back(&constant_store, operand.constant);
					switch(operand.constant.type){
					case TYPE_NUMBER:
						add_instruction_operate_with_int_const(instr, current_rule);
						break;
					case TYPE_REAL:
						add_instruction_operate_with_real_const(instr, current_rule);
						break;
					case TYPE_STRING:
						add_instruction_operate_with_string_const(instr, current_rule);
						break;
					}
					vector_ExprVar_push_back(&expr_stack, top);
					break;
				case OPERAND_COMPUTED:
					//we have to increment the stack pointer as the operation will pop one variable and replace the other.
					current_stack_top_index++;
				case OPERAND_STACK:
					top.variable.type=get_result_type(top.variable.type, current_rule);
					switch(current_rule){
					case RULE_EXPR__ADD:
						instr.fPtr=inst_add;
						break;
					case RULE_EXPR__SUB:
						instr.fPtr=inst_sub;
						break;
					case RULE_EXPR__DIV:
						instr.fPtr=inst_div;
						break;
					case RULE_EXPR__MULT:
						instr.fPtr=inst_mult;
						break;
					case RULE_EXPR__EQ:
						instr.fPtr=inst_logic_eq;
						top.variable.type=SEMVAR_NUMBER;
						break;
					case RULE_EXPR__GREATER:
						instr.fPtr=inst_logic_gr;
						top.variable.type=SEMVAR_NUMBER;
						break;
					case RULE_EXPR__GREATEREQ:
						instr.fPtr=inst_logic_greq;
						top.variable.type=SEMVAR_NUMBER;
						break;
					case RULE_EXPR__LESS:
						instr.fPtr=inst_logic_le;
						top.variable.type=SEMVAR_NUMBER;
						break;
					case RULE_EXPR__LESSEQ:
						instr.fPtr=inst_logic_leeq;
						top.variable.type=SEMVAR_NUMBER;
						break;
					case RULE_EXPR__NEQ:
						instr.fPtr=inst_logic_neq;
						top.variable.type=SEMVAR_NUMBER;
						break;
					default:
						fprintf(stderr, "Unexpected rule in operator expression\n");
						Throw(RET_INTERN);
					}
					add_instruction(instr);
					vector_ExprVar_push_back(&expr_stack, top);
					//we always 
					break;
				}
			}

		}
		//the expr ended
		assert(vector_ExprVar_count(&expr_stack)==1);
		*result=vector_ExprVar_pop_back(&expr_stack);
	}
	Catch(retval){
		current_stack_top_index+=vector_ExprVar_count(&expr_stack);
		if(retval==RET_INTERN){
			for(size_t i=0; i<vector_ExprVar_count(&expr_stack); i++){
				ExprVar exvar=vector_ExprVar_pop_back(&expr_stack);
				if(exvar.type==EXVAR_CONST&&exvar.constant.type==TYPE_STRING){
					free(exvar.constant.string);
				}
			}
			vector_ExprVar_destroy(&expr_stack);
			vector_unsigned_destroy(&param_counts);
			Throw(retval);
		}
		//otherwise just clean up and return.
		set_real_retval(retval);
	}
	if(real_retval!=RET_EOK){
		for(size_t i=0; i<vector_ExprVar_count(&expr_stack); i++){
			ExprVar exvar=vector_ExprVar_pop_back(&expr_stack);
			if(exvar.type==EXVAR_CONST&&exvar.constant.type==TYPE_STRING){
				free(exvar.constant.string);
			}
		}
	}
	vector_ExprVar_destroy(&expr_stack);
	vector_unsigned_destroy(&param_counts);
	return real_retval;
}

void goto_next_expected_rule(ParseTree *parseTree, Rule *expectedRules, int expectedRuleCount){
	while(true){
		current_rule_index++;
		assert(current_rule_index<vector_Rule_count(&parseTree->rules));
		if(current_rule==RULE_FUNC_DECL_LIST__EPSILON){
			current_rule_index--;
			return;
		}
		for(int i=0; i<expectedRuleCount; i++){
			if(current_rule==expectedRules[i]){
				current_rule_index--;
				return;
			}
		}
		//if current rule wasn't expected, check if it has a value and advance values accordingly
		switch(current_rule){
		case RULE_EXPR__LEX_STRING_LITERAL:
		case RULE_TERM__STRING:
		case RULE_FUNC_DECL:
		case RULE_FUNC_DECL_ARG_LIST_CONT__COMMA_DECL_ARG_CONT:
		case RULE_FUNC_DECL_ARG_LIST__DECL_ARG_CONT:
		case RULE_ASSIGNMENT:
		case RULE_TERM__REAL:
		case RULE_TERM__ID:
		case RULE_TERM__NUMBER:
		case RULE_VAR_DEF__AUTO_ID_VAR_INIT:
		case RULE_VAR_DEF__TYPE_ID_OPT_VAR_INIT:
		case RULE_EXPR__FUNC_NO_PARAM:
		case RULE_EXPR__FUNC_ONE_PARAM:
		case RULE_EXPR__FUNC_PARAM_LIST:
		case RULE_EXPR__LEX_ID:
		case RULE_EXPR__LEX_NUMBER:
		case RULE_EXPR__LEX_REAL_NUMBER:
			current_value_index++;
			break;
		default:
			break;
		}
	}
}

int analyze_variable_declaration(ParseTree *parseTree){
	ExprVar initial_value;
	VarType nvar_type;
	Token name=next_value;
	int retval=RET_EOK;
	int real_retval=RET_EOK;
	if(next_rule==RULE_VAR_DEF__TYPE_ID_OPT_VAR_INIT){
		nvar_type=translate_semvar_type_to_type(translate_rule_to_semvar_type(next_rule));
		if(next_rule==RULE_OPT_VAR_INIT__VAR_INIT){
			current_rule_index++;
			assert(current_rule==RULE_VAR_INIT__ASSIGN_EXPR);
			if((retval=analyze_expression(parseTree, &initial_value))!=RET_EOK){
				return retval;
			}
			Try{
				//if the variable isn't on top of the stack after the conversion
				if(!exvar_convert_to_type(nvar_type, &initial_value)){
					if(initial_value.type==EXVAR_CONST){
						add_instruction_push_const(&initial_value);
					}
					else{
						//must be a stackvar
						add_instruction_copy_stackvar(&initial_value);
					}
				}
			}
			Catch(retval){
				if(retval==RET_INTERN){
					Throw(retval);
				}
				set_real_retval(retval);
				//if it's just a semantic error, create the variable so the rest of translation doesn't throw as much of a fit.
			}
			//now that a variable of the correct type and value is at the top of the stack, add the semantic variable
			retval=create_new_semantic_variable(translate_type_to_semvar_type(nvar_type), name.IDName);
			if(retval!=RET_EOK){
				set_real_retval(retval);
				return real_retval;
			}
		}
		else{
			//there is no initialization
			assert(current_rule==RULE_OPT_VAR_INIT__EPSILON);
			SemInstr instr;
			instr.stack_adjustment=-1;
			instr.top_right=false;
			instr.rPtrIndex=SIZE_MAX;
			instr.stack_op=nvar_type;
			instr.const_op=SIZE_MAX;
			instr.fPtr=inst_add_variable;
			add_instruction(instr);
			current_stack_top_index--;
			retval=create_new_semantic_variable(translate_type_to_semvar_type(nvar_type), name.IDName);
			if(retval!=RET_EOK){
				return retval;
			}
		}
	}
	else{
		assert(current_rule==RULE_VAR_DEF__AUTO_ID_VAR_INIT);
		if(next_rule==RULE_OPT_VAR_INIT__EPSILON){
			return RET_DERIVE;
		}
		assert(current_rule==RULE_OPT_VAR_INIT__VAR_INIT);
		current_rule_index++;
		assert(current_rule==RULE_VAR_INIT__ASSIGN_EXPR);
		if((retval=analyze_expression(parseTree, &initial_value))!=RET_EOK){
			return retval;
		}
		nvar_type=get_type_of_exvar(initial_value);
		if(initial_value.type==EXVAR_CONST){
			add_instruction_push_const(&initial_value);
		}
		else if(initial_value.type==EXVAR_STACKVAR){
			add_instruction_copy_stackvar(&initial_value);
		}
		//if it's a compvar we don't need to do anything
		retval=create_new_semantic_variable(translate_type_to_semvar_type(nvar_type), name.IDName);
		if(retval!=RET_EOK){
			return retval;
		}
	}
	return real_retval;
}

int analyze_assignment(ParseTree *parseTree){
	int retval;
	current_rule_index++;
	assert(current_rule==RULE_ASSIGNMENT);
	assert(vector_Token_at(&parseTree->values, current_value_index+1)->type==LEX_ID);
	SemVar *dest_var=htab_search(&symbol_table, next_value.IDName);
	if(!dest_var){
		error_message("Undefined identifier \'%s\'", current_value.line, current_value.IDName);
		return RET_SEM_DEF;
	}
	SemInstr instr;
	instr.top_right=false;
	instr.stack_op=dest_var->stack_local_pos;
	instr.const_op=SIZE_MAX;
	instr.rPtrIndex=SIZE_MAX;
	instr.stack_adjustment=1;
	ExprVar assigned_expr;
	retval=analyze_expression(parseTree, &assigned_expr);
	if(retval!=RET_EOK){
		return retval;
	}
	Try{
		if(!exvar_convert_to_type(translate_semvar_type_to_type(dest_var->type), &assigned_expr)){
			if(assigned_expr.type==EXVAR_CONST){
				instr.const_op=vector_SemConst_count(&constant_store);
				vector_SemConst_push_back(&constant_store, assigned_expr.constant);
				instr.fPtr=inst_assign_const;
				instr.stack_adjustment=0;
				add_instruction(instr);
			}
			else{
				//must be a stackvar
				add_instruction_copy_stackvar(&assigned_expr);
				instr.fPtr=inst_assign;
				add_instruction(instr);
				current_stack_top_index++;
			}
		}
		else{
			instr.fPtr=inst_assign;
			add_instruction(instr);
			current_stack_top_index++;
		}
	}
	Catch(retval){
		if(assigned_expr.type==EXVAR_CONST&&assigned_expr.constant.type==TYPE_STRING){
			free(assigned_expr.constant.string);
		}
		if(retval==RET_INTERN){
			Throw(retval);
		}
		return retval;
	}
	return RET_EOK;
}

//assumes that the if clause is already done and the conditional jump's destination is set
int analyze_else_clause(ParseTree *parseTree){
	assert(current_rule==RULE_ELSE_CLAUSE__ELSE_STATEMENT);
	//make sure that the last ended scope is an if
	while(last_ended_scope_type!=SCOPE_IF){
		assert(!(current_scope->type&SCOPE_COMPOSITE_STATEMENT));
		int retval=end_scope(parseTree);
		if(retval!=RET_EOK){
			return retval;
		}
	}
	//the previous instruction is the jump generated at the end of the if block
	//so make sure that it will point to the end of the else instead of the next
	//instruction.
	create_new_scope(SCOPE_ELSE, vector_SemInstr_count(&inst_vect)-1, SIZE_MAX);
	last_ended_scope_type=SCOPE_ELSE;
	return RET_EOK;
}

//the jump instruction will be at the end of the instruction vector after this function finishes
int analyze_condition(ParseTree *parseTree){
	int retval=RET_EOK;
	ExprVar cond_expr;
	retval=analyze_expression(parseTree, &cond_expr);
	if(retval!=RET_EOK){
		return retval;
	}
	Try{
		SemInstr instr;
		instr.top_right=false;
		instr.const_op=SIZE_MAX;
		instr.stack_adjustment=0;
		instr.rPtrIndex=SIZE_MAX;
		if(!exvar_convert_to_type(TYPE_NUMBER, &cond_expr)){
			if(cond_expr.type==EXVAR_CONST){
				//TODO: Maybe make sure that the dead code will not be generated
				if(cond_expr.constant.number){
					//if the constant is nonzero, always continue into the if block.
					instr.fPtr=inst_nop;
				}
				else{
					//if the constant is zero, always go to else (or past the if block if there is no else.
					instr.stack_op=0;
					instr.fPtr=inst_jmp;
				}
			}
			else{
				//must be a stackvar
				instr.fPtr=inst_condition;
				instr.stack_op=cond_expr.variable.stack_local_pos;
			}
		}
		else{
			//-1 means temporary variable on top of the stack
			instr.stack_op=1;
			//will pop the tempvar off the stack
			instr.stack_adjustment=1;
			//the 1 must be made into -1
			instr.top_right=true;
			instr.fPtr=inst_condition;
			current_stack_top_index++;
		}
		add_instruction(instr);
	}
	Catch(retval){
		if(cond_expr.type==EXVAR_CONST&&cond_expr.constant.type==TYPE_STRING){
			free(cond_expr.constant.string);
		}
		if(retval==RET_INTERN){
			Throw(retval);
		}
		return retval;
	}
	return retval;
}

int analyze_if_statement(ParseTree *parseTree){
	current_rule_index++;
	assert(current_rule==RULE_IF_STATEMENT);
	int retval=analyze_condition(parseTree);
	if(retval!=RET_EOK){
		return retval;
	}
	create_new_scope(SCOPE_IF, vector_SemInstr_count(&inst_vect)-1, SIZE_MAX);
	return RET_EOK;
}

int analyze_for_statement(ParseTree *parseTree){
	//If the program looks like for([1]; [2]; [3]), the instructions will be like this:
	//[1]
	//jump to [2]
	//[3]
	//if([2]) jump to end
	//cycle body
	//jump to [3]
	current_rule_index++;
	assert(current_rule==RULE_FOR_STATEMENT);
	int retval=RET_EOK;
	//the pointers will be filled in later
	create_new_scope(SCOPE_FOR, SIZE_MAX, SIZE_MAX);
	//create [1]
	retval=analyze_variable_declaration(parseTree);
	if(retval!=RET_EOK){
		return retval;
	}	
	SemInstr instr;
	instr.const_op=SIZE_MAX;
	instr.stack_op=0;
	instr.rPtrIndex=SIZE_MAX;
	instr.stack_adjustment=0;
	instr.top_right=false;
	instr.fPtr=inst_jmp;
	//we will put the assignment in front of the condition and jump over it the first time
	//this will be jump to [2]
	size_t jump_to_cond_index=vector_SemInstr_count(&inst_vect);
	add_instruction(instr);
	//so the cycle starts here
	current_scope->cycle_start_index=jump_to_cond_index+1;
	//analyze the condition ([2])
	retval=analyze_condition(parseTree);
	if(retval!=RET_EOK){
		current_scope->jump_index=0;
		return retval;
	}
	Vector_SemInstr tempStore;
	size_t condInstCount=vector_SemInstr_count(&inst_vect)-jump_to_cond_index-1;
	retval=vector_SemInstr_init_sized(&tempStore, condInstCount);
	if(retval!=RET_EOK){
		return RET_INTERN;
	}
	//put [2] somewhere else so that we can insert [3] before it
	for(size_t currentCount=0; currentCount<condInstCount; currentCount++){
		//put all the new instructions into temporary storage
		vector_SemInstr_push_back(&tempStore, vector_SemInstr_pop_back(&inst_vect));
	}
	//analyze the assignment ([3])
	retval=analyze_assignment(parseTree);
	if(retval!=RET_EOK){
		vector_SemInstr_destroy(&tempStore);
		current_scope->jump_index=0;
		return retval;
	}
	//set the jump so it points to [2]
	vector_SemInstr_at(&inst_vect, jump_to_cond_index)->rPtrIndex=vector_SemInstr_count(&inst_vect);
	//and put [2] there.
	for(size_t currentCount=0; currentCount<condInstCount; currentCount++){
		//now put the condition instructions back. Assignment leaves the stack as it was before
		//so no adjustments are necessary
		vector_SemInstr_push_back(&inst_vect, vector_SemInstr_pop_back(&tempStore));
	}
	vector_SemInstr_destroy(&tempStore);
	//set the condition instruction as the jump
	current_scope->jump_index=vector_SemInstr_count(&inst_vect)-1;
	return retval;
}	

int analyze_while_statement(ParseTree *parseTree){
	current_rule_index+=2;
	assert(current_rule==RULE_WHILE_CLAUSE);
	//the cycle starts at the next instruction
	create_new_scope(SCOPE_WHILE, 0, vector_SemInstr_count(&inst_vect));
	int retval=analyze_condition(parseTree);
	if(retval!=RET_EOK){
		return retval;
	}
	current_scope->jump_index=vector_SemInstr_count(&inst_vect)-1;
	return RET_EOK;
}

int analyze_do_statement(ParseTree *parseTree){
	current_rule_index++;
	assert(current_rule==RULE_DO_WHILE);
	create_new_scope(SCOPE_DO_WHILE, SIZE_MAX, vector_SemInstr_count(&inst_vect));
	return RET_EOK;
}

int analyze_return_statement(ParseTree *parseTree, UserFn *currentFunc){
	current_rule_index++;
	assert(current_rule==RULE_RETURN_STATEMENT);
	ExprVar ret_expr;
	SemInstr instr;
	//we will add the expression to the top of the stack
	instr.stack_op=-current_stack_top_index+1;
	instr.fPtr=inst_ret;
	instr.stack_adjustment=1;
	instr.const_op=SIZE_MAX;
	instr.rPtrIndex=SIZE_MAX;
	instr.top_right=false;
	int retval=analyze_expression(parseTree, &ret_expr);
	if(retval!=RET_EOK){
		return retval;
	}
	Try{
		if(!exvar_convert_to_type(translate_semvar_type_to_type(currentFunc->return_type), &ret_expr)){
			if(ret_expr.type==EXVAR_CONST){
				add_instruction_push_const(&ret_expr);
			}
			else{
				//stackvar
				add_instruction_copy_stackvar(&ret_expr);
			}
		}
		//if it's an exvar, we're all set
	}
	Catch(retval){
		if(ret_expr.type==EXVAR_CONST&&ret_expr.constant.type==TYPE_STRING){
			free(ret_expr.constant.string);
		}
		if(retval==RET_INTERN){
			Throw(retval);
		}
		return retval;
	}
	add_instruction(instr);
	current_stack_top_index++;
	return retval;
}

int analyze_input_statement(ParseTree *parseTree){
	current_rule_index++;
	assert(current_rule==RULE_INPUT);
	SemVar *target_variable;
	SemInstr instr;
	instr.stack_adjustment=0;
	instr.const_op=SIZE_MAX;
	instr.top_right=false;
	instr.rPtrIndex=SIZE_MAX;
	do{
		target_variable=htab_search(&symbol_table, next_value.IDName);
		if(!target_variable){
			error_message("undefined identifier \'%s\'", current_value.line, current_value.IDName);
			return RET_SEM_DEF;
		}
		instr.stack_op=target_variable->stack_local_pos;
		switch(target_variable->type){
		case SEMVAR_REAL:
			instr.fPtr=inst_input_real;
			break;
		case SEMVAR_NUMBER:
			instr.fPtr=inst_input_number;
			break;
		case SEMVAR_STRING:
			instr.fPtr=inst_input_string;
			break;
		case SEMVAR_FUNC:
			print_customized_message("Cannot input into functions.");
			return RET_SEM_EXP;
		}
		add_instruction(instr);
	}while(next_rule==RULE_INPUT_CHAIN__ID_INPUT_CHAIN);
	assert(current_rule==RULE_INPUT_CHAIN__EPSILON);
	return RET_EOK;
}

int analyze_output_statement(ParseTree *parseTree){
	current_rule_index++;
	assert(current_rule==RULE_OUTPUT);
	SemConst constant;
	SemVar *stackvar;
	SemInstr instr;
	instr.stack_adjustment=0;
	instr.rPtrIndex=SIZE_MAX;
	instr.top_right=false;
	do{
		stackvar=NULL;
		instr.stack_op=0;
		instr.const_op=SIZE_MAX;
		switch(next_rule){
		case RULE_TERM__REAL:
			constant.type=TYPE_REAL;
			constant.real=next_value.real;
			break;
		case RULE_TERM__ID:
			stackvar=htab_search(&symbol_table, next_value.IDName);
			if(!stackvar){
				error_message("undefined identifier \'%s\'", current_value.line, current_value.IDName);
				return RET_SEM_DEF;
			}
			break;
		case RULE_TERM__NUMBER:
			constant.type=TYPE_NUMBER;
			constant.number=next_value.number;
			break;
		case RULE_TERM__STRING:
			constant.type=TYPE_STRING;
			constant.string=malloc(sizeof(VarString)+strlen(next_value.string)+1);
			if(!constant.string){
				Throw(RET_INTERN);
			};
			strcpy(constant.string->data, current_value.string);
			constant.string->references=1;
			constant.string->length=strlen(constant.string->data);
			break;
		default:
			fprintf(stderr, "unexpected rule in term type switch\n");
		}
		ExprVar exvar;
		if(stackvar){
			exvar.type=EXVAR_STACKVAR;
			instr.stack_op=stackvar->stack_local_pos;
			exvar.variable=*stackvar;
		}
		else{	
			exvar.type=EXVAR_CONST;
			exvar.constant=constant;
			instr.const_op=vector_SemConst_count(&constant_store);
			vector_SemConst_push_back(&constant_store, constant);
		}
		switch(get_type_of_exvar(exvar)){
		case TYPE_STRING:
			instr.fPtr=inst_output_string;
			break;
		case TYPE_NUMBER:
			instr.fPtr=inst_output_number;
			break;
		case TYPE_REAL:
			instr.fPtr=inst_output_real;
			break;
		}
		add_instruction(instr);
	}while(next_rule==RULE_OUTPUT_CHAIN__TERM_OUTPUT_CHAIN);
	assert(current_rule==RULE_OUTPUT_CHAIN__EPSILON);
	return RET_EOK;
}

int analyze_statement(ParseTree *parseTree, UserFn *currentFunc){
	switch(current_rule){
	case RULE_STATEMENT__ASSIGNMENT:
		return analyze_assignment(parseTree);
	case RULE_STATEMENT__VAR_DEF:
		return analyze_variable_declaration(parseTree);
	case RULE_STATEMENT__RETURN_STATEMENT:
		return analyze_return_statement(parseTree, currentFunc);
	case RULE_STATEMENT__INPUT:
		return analyze_input_statement(parseTree);
	case RULE_STATEMENT__OUTPUT:
		return analyze_output_statement(parseTree);
	case RULE_STATEMENT__WHILE_STATEMENT:
		return analyze_while_statement(parseTree);
	case RULE_STATEMENT__DO_WHILE:
		return analyze_do_statement(parseTree);
	case RULE_STATEMENT__IF_STATEMENT:
		return analyze_if_statement(parseTree);
	case RULE_STATEMENT__FOR_STATEMENT:
		return analyze_for_statement(parseTree);
	case RULE_ELSE_CLAUSE__EPSILON:
		return RET_EOK;
	case RULE_ELSE_CLAUSE__ELSE_STATEMENT:
		return analyze_else_clause(parseTree);
	case RULE_STATEMENT__COMPOSITE_STATEMENT:
		//if this happens in a braced scope, create new scope
		if(current_scope->type&SCOPE_COMPOSITE_STATEMENT){
			create_new_scope(SCOPE_OTHER|SCOPE_COMPOSITE_STATEMENT, SIZE_MAX, SIZE_MAX);
		}
		//if it's in an unbraced scope (like if without braces), then this is the
		//first statement after the one that started the scope, so just set 
		//the composite statement flag
		else{
			current_scope->type|=SCOPE_COMPOSITE_STATEMENT;
		}
		current_rule_index++;
		return RET_EOK;
	case RULE_WHILE_CLAUSE:
		//end_scope expects that the next rule is WHILE_CLAUSE in case of do while scope
		current_rule_index--;
		do{
			int retval=end_scope(parseTree);
			if(retval!=RET_EOK){
				return retval;
			}
		}while(last_ended_scope_type!=SCOPE_DO_WHILE);
		return RET_EOK;
	default:
		fprintf(stderr, "unexpected rule in nonscoped statement analysis\n");
		Throw(RET_INTERN);
		return RET_INTERN;
	}
}

void handle_statement_error(ParseTree *parseTree){
	Rule expectedRules[]={
		RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		RULE_STATEMENT_LIST__EPSILON,
		RULE_STATEMENT__COMPOSITE_STATEMENT
	};
	goto_next_expected_rule(parseTree, expectedRules, sizeof(expectedRules)/sizeof(expectedRules[0]));
}

void free_function_table(){
	for(size_t i=0; i<vector_UserFn_count(&function_table); i++){
		vector_FuncParam_destroy(&vector_UserFn_at(&function_table, i)->params);
	}
	vector_UserFn_destroy(&function_table);
}

void add_const_func_to_symbol_table(SemVarType ret_type, char *name, SemVar funcvar, int num_params, ...){
	va_list args;
	va_start(args, num_params);
	htab_insert(&symbol_table, name, funcvar);
	UserFn *func;
	vector_UserFn_push_back(&function_table, (UserFn){.return_type=ret_type, .first_inst_index=SIZE_MAX});
	func=vector_UserFn_back(&function_table);
	if(vector_FuncParam_init(&func->params)!=RET_EOK){
		Throw(RET_INTERN);
	}
	for(int i=0; i<num_params; i++){
		FuncParam param;
		param.type=va_arg(args, SemVarType);
		param.name=va_arg(args, char*);
		vector_FuncParam_push_back(&func->params, param);
	}
	va_end(args);
}

int analyze_program_parse_tree(ParseTree *parseTree){
	current_rule_index=SIZE_MAX;
	current_value_index=SIZE_MAX;
	assert(vector_Rule_count(&parseTree->rules)>0);
	current_stack_top_index=0;
	UserFn *current_fn;
	SemInstr instr;
	instr.stack_op=0;
	instr.fPtr=inst_nop;
	instr.top_right=false;
	instr.const_op=SIZE_MAX;
	instr.rPtrIndex=SIZE_MAX;
	instr.stack_adjustment=0;
	int retval=RET_EOK;
	int real_retval=RET_EOK;
	enum {
		INIT_NONE,
		INIT_FUNCTIONS,
		INIT_INSTRUCTIONS,
		INIT_CONSTANTS,
		INIT_SYMBOLS, 
	}initializedVariables=INIT_NONE;
	retval=vector_SemConst_init(&constant_store);
	if(retval!=RET_EOK){
		set_real_retval(RET_INTERN);
		goto init_fail_cleanup;
	}
	initializedVariables=INIT_CONSTANTS;
	retval=vector_UserFn_init(&function_table);
	if(retval!=RET_EOK){
		set_real_retval(RET_INTERN);
		goto init_fail_cleanup;
	}
	initializedVariables=INIT_FUNCTIONS;
	retval=vector_SemInstr_init(&inst_vect);
	if(retval!=RET_EOK){
		set_real_retval(RET_INTERN);
		goto init_fail_cleanup;
	}
	initializedVariables=INIT_INSTRUCTIONS;
	retval=htab_init_sized(&symbol_table, 4096);
	if(retval!=RET_EOK){
		set_real_retval(RET_INTERN);
		goto init_fail_cleanup;
	}
	initializedVariables=INIT_SYMBOLS;
	size_t rule_count=vector_Rule_count(&parseTree->rules);
	Try{
		add_const_func_to_symbol_table(SEMVAR_NUMBER, "length", (SemVar){.stack_local_pos=0, .type=SEMVAR_FUNC}, 1,
				SEMVAR_STRING, "s");
		add_const_func_to_symbol_table(SEMVAR_STRING, "substr", (SemVar){.stack_local_pos=1, .type=SEMVAR_FUNC}, 3,
				SEMVAR_STRING, "s", SEMVAR_NUMBER, "i", SEMVAR_NUMBER, "n");
		add_const_func_to_symbol_table(SEMVAR_STRING, "concat", (SemVar){.stack_local_pos=2, .type=SEMVAR_FUNC}, 2,
				SEMVAR_STRING, "s1", SEMVAR_STRING, "s2");
		add_const_func_to_symbol_table(SEMVAR_NUMBER, "find", (SemVar){.stack_local_pos=3, .type=SEMVAR_FUNC}, 2,
				SEMVAR_STRING, "s", SEMVAR_STRING, "search");
		add_const_func_to_symbol_table(SEMVAR_STRING, "sort", (SemVar){.stack_local_pos=4, .type=SEMVAR_FUNC}, 1,
				SEMVAR_STRING, "s");
		add_const_func_to_symbol_table(SEMVAR_NUMBER, "main", (SemVar){.stack_local_pos=5, .type=SEMVAR_FUNC}, 0);
		while(++current_rule_index!=rule_count){
			switch(current_rule){
			case RULE_FUNC_DECL_LIST__FUNC_DECL_FUNC_DECL_LIST:
				current_fn=analyze_func_decl(parseTree);
				if(!current_fn){
					set_real_retval(RET_SEM_DEF);
					//go to next func declaration.
					Rule expectedRules[]={
						RULE_FUNC_DECL_LIST__EPSILON,
						RULE_FUNC_DECL_LIST__FUNC_DECL_FUNC_DECL_LIST
					};
					goto_next_expected_rule(parseTree, expectedRules, 2);
				}
				break;
			case RULE_FUNC_DECL_LIST__EPSILON:
				assert(current_rule_index==rule_count-1);
				break;
			case RULE_OPT_FUNC_BODY__SEMICOLON:
				break;
			case RULE_OPT_FUNC_BODY__COMPOSITE_STATEMENT:
				assert(current_stack_top_index==0||real_retval!=RET_EOK);
				//create a scope
				if(current_fn->first_inst_index!=SIZE_MAX){
					error_message("Redefinition of a function", current_value.line);
					set_real_retval(RET_SEM_DEF);
					//go to next func declaration.
					Rule expectedRules[]={
						RULE_FUNC_DECL_LIST__EPSILON,
						RULE_FUNC_DECL_LIST__FUNC_DECL_FUNC_DECL_LIST
					};
					goto_next_expected_rule(parseTree, expectedRules, 2);
					break;
				}
				create_new_scope(SCOPE_FUNC|SCOPE_COMPOSITE_STATEMENT, SIZE_MAX, SIZE_MAX);
				current_fn->first_inst_index=vector_SemInstr_count(&inst_vect);
				size_t param_count=vector_FuncParam_count(&current_fn->params);
				//the stack top must be initialized with the number of parameters
				instr.stack_adjustment=-(Position)param_count;
				add_instruction(instr);
				//create the parameters.
				for(size_t i=0; i<param_count; i++){
					FuncParam *param=vector_FuncParam_at(&current_fn->params, i);
					//it can't fail here, the parameters are already checked for duplication
					current_stack_top_index--;
					retval=create_new_semantic_variable(param->type, param->name);
					assert(retval==RET_EOK);
				}
				current_rule_index++;
				assert(current_rule==RULE_COMPOSITE_STATEMENT);
				break;
			case RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST:
				//this rule is never invoked after a scope creating statement
				//because they have their own statement after them.
				//So until you find a scope "protected" by braces, delete scopes
				//no need to check current scope for null, as
				//function scope is always brace-enclosed.
				while(!(current_scope->type&SCOPE_COMPOSITE_STATEMENT)){
					retval=end_scope(parseTree);
					if(retval!=RET_EOK){
						set_real_retval(retval);
						handle_statement_error(parseTree);
					}
				}
				break;
			case RULE_STATEMENT_LIST__EPSILON:
				//end all non-braced scopes
				while(!(current_scope->type&SCOPE_COMPOSITE_STATEMENT)){
					retval=end_scope(parseTree);
					if(retval!=RET_EOK){
						set_real_retval(retval);
						handle_statement_error(parseTree);
					}
				}
				//end the scope of the current composite statement
				//"enclosing" scopes without braces will be ended
				//(if necessary) by the next statement_list rule
				retval=end_scope(parseTree);
				if(retval!=RET_EOK){
					set_real_retval(retval);
					handle_statement_error(parseTree);
				}
				assert(current_scope
						||*vector_Rule_at(&parseTree->rules, current_rule_index+1)
						==RULE_FUNC_DECL_LIST__FUNC_DECL_FUNC_DECL_LIST
						||*vector_Rule_at(&parseTree->rules, current_rule_index+1)
						==RULE_FUNC_DECL_LIST__EPSILON);
				break;
			case RULE_WHILE_CLAUSE:
			case RULE_STATEMENT__ASSIGNMENT:
			case RULE_STATEMENT__VAR_DEF:
			case RULE_STATEMENT__INPUT:
			case RULE_STATEMENT__OUTPUT:
			case RULE_STATEMENT__WHILE_STATEMENT:
			case RULE_STATEMENT__DO_WHILE:
			case RULE_STATEMENT__RETURN_STATEMENT:
			case RULE_STATEMENT__IF_STATEMENT:
			case RULE_STATEMENT__FOR_STATEMENT:
			case RULE_ELSE_CLAUSE__ELSE_STATEMENT:
			case RULE_ELSE_CLAUSE__EPSILON:
			case RULE_STATEMENT__COMPOSITE_STATEMENT:
				retval=analyze_statement(parseTree, current_fn);
				if(retval!=RET_EOK){
					set_real_retval(retval);
					handle_statement_error(parseTree);
				}
				break;
			default:
				fprintf(stderr, "unexpected rule in main semantic analysis\n");
				Throw(RET_INTERN);
			}
		}
	}
	Catch(retval){
		set_real_retval(retval);
	}
	vector_Rule_destroy(&parseTree->rules);
	htab_destroy(&symbol_table);
	size_t token_count=vector_Token_count(&parseTree->values);
	for(size_t i=0; i<token_count; i++){
		Token *tok=vector_Token_at(&parseTree->values, i);
		//only free IDs, strings are in the constant vector
		if(tok->type==LEX_ID){
			free(tok->IDName);
		}
		else if(tok->type==LEX_STRING_LITERAL){
			free(tok->string);
		}
	}
	vector_Token_destroy(&parseTree->values);
	graphTree=NULL;
	program=NULL;
	if(real_retval==RET_EOK){
		size_t inst_count=vector_SemInstr_count(&inst_vect);
		program=malloc(sizeof(Graph)*inst_count);
		if(!program){
			set_real_retval(RET_INTERN);
			goto end_fail_cleanup;
		}
		assert(current_stack_top_index==0);
		for(size_t i=0; i<inst_count; i++){
			SemInstr *sinstr=vector_SemInstr_at(&inst_vect, i);
			Graph *instr=program+i;
			instr->fPtr=sinstr->fPtr;
			instr->src=sinstr->const_op==SIZE_MAX?NULL:(VarValues*)&vector_SemConst_at(&constant_store, sinstr->const_op)->string;
			//This is defined behavior. Quoting the C11 standard, §6.7.2.1 16:
			//"A pointer to a union object, suitably converted, points to each of its members
			//(or if a member is a bit-field, then to the unit in which it resides), and vice versa."
			instr->rPtr=sinstr->rPtrIndex==SIZE_MAX?NULL:program+sinstr->rPtrIndex;
			//make the absolute addresses relative to the top of the stack
			instr->dest=sinstr->stack_op-(sinstr->stack_op>=0?0:current_stack_top_index);
			//negate the dest if it's an arithmetic operation with reversed operands.
			instr->dest=sinstr->top_right?-instr->dest:instr->dest;
			current_stack_top_index+=sinstr->stack_adjustment;
		}
		graphTree=malloc(sizeof(GraphTree)*(vector_UserFn_count(&function_table)-FIRST_USER_FUNC));
		if(graphTree==NULL){
			set_real_retval(RET_INTERN);
			goto end_fail_cleanup;
		}
		vector_SemInstr_destroy(&inst_vect);
		for(size_t i=FIRST_USER_FUNC; i<vector_UserFn_count(&function_table); i++){
			size_t first_inst_index=vector_UserFn_at(&function_table, i)->first_inst_index;
			if(first_inst_index==SIZE_MAX){
				print_customized_message("A function was not defined");
				set_real_retval(RET_SEM_DEF);
			}
			graphTree[i-FIRST_USER_FUNC]=(GraphTree){.start=program+first_inst_index};
		}
		if(real_retval!=RET_EOK){
			goto end_fail_cleanup;
		}
	}
	else{
		vector_SemInstr_destroy(&inst_vect);
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
	free_function_table();
	return real_retval;
end_fail_cleanup:
	if(program){
		free(program);
	}
	if(graphTree){
		free(graphTree);
		graphTree=NULL;
	}
	//only destroy instruction vector when failure happened before graphTree allocation
	else{
		vector_SemInstr_destroy(&inst_vect);
	}
	free_function_table();
	size_t constcount=vector_SemConst_count(&constant_store);
	for(size_t i=0; i<constcount; i++){
		SemConst *constant=vector_SemConst_at(&constant_store, i);
		if(constant->type==TYPE_STRING){
			assert(constant->string->references==1);
			varstring_delete(constant->string);
		}
	}
	vector_SemConst_destroy(&constant_store);
	return real_retval;
init_fail_cleanup:
	switch(initializedVariables){
	case INIT_SYMBOLS:
		htab_destroy(&symbol_table);
	case INIT_INSTRUCTIONS:
		vector_SemInstr_destroy(&inst_vect);
	case INIT_FUNCTIONS:
		vector_UserFn_destroy(&function_table);
	case INIT_CONSTANTS:
		//constants will be destroyed in the global free
	case INIT_NONE:
		break;
	}
	size_t valcount=vector_Token_count(&parseTree->values);
	for(size_t i=0; i<valcount; i++){
		Token *tmp=vector_Token_at(&parseTree->values, i);
		if(tmp->type==LEX_ID){
			free(tmp->IDName);
		}
		else if(tmp->type==LEX_STRING_LITERAL){
			free(tmp->string);
		}
	}
	return real_retval;
}
