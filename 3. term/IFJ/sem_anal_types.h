/*
 * sem_anal_types.h
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

#ifndef SEM_ANAL_TYPES_H_
#define SEM_ANAL_TYPES_H_
#include "varstack.h"
#include "stdint.h"

typedef enum {
	SEMVAR_NUMBER,
	SEMVAR_STRING,
	SEMVAR_REAL,
	SEMVAR_FUNC
}SemVarType;

#if SIZE_MAX==18446744073709551615ULL
typedef int64_t Position;
#else
typedef int32_t Position;
#endif

typedef struct{
	Position stack_local_pos;
	SemVarType type;
}SemVar;

typedef char *String;

typedef struct{
	String name;
	SemVarType type;
}FuncParam;

typedef struct{
	VarType type;
	union{
		VarString *string;
		int number;
		double real;
	};
}SemConst;

typedef struct{
	bool (*fPtr)(int, void*);
	Position stack_op;
	size_t const_op;
	size_t rPtrIndex;
	int stack_adjustment;
	bool top_right;
}SemInstr;

typedef enum{
	EXVAR_STACKVAR,
	EXVAR_CONST,
	EXVAR_COMPUTED_VAR
}ExprVarType;

typedef struct{
	ExprVarType type;
	size_t current_instr_pos;
	union{
		SemVar variable;
		SemConst constant;
	};
}ExprVar;

#endif //SEM_ANAL_TYPES_H_
