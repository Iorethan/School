/*
 * synt_anal.c
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

#include <assert.h>
#include "error.h"
#include "synt_anal.h"
#include "cexception.h"
#include "token.h"
#include "synt_anal_types.h"
#include "vector_syntacticelement.h"
#include "lex_anal.h"

#define SYN_EL_TERM(lex_type) {.type=SYNT_TERMINAL, .terminal=lex_type},
#define SYN_EL_NTERM(nterm) {.type=SYNT_NONTERMINAL, .nonterminal=nterm},
#define SYN_EL_NONE {.type=SYNT_NONE}

#define GOTO_CLEANUP_EXCEPTION 0xbadf00d

#define MAX_HANDLE_LENGTH 10

//the right sides are in reverse order until EXPR rules
const SyntacticElement rules[][MAX_HANDLE_LENGTH+1]={//+1 because we need a stopper
	[RULE_FUNC_DECL_LIST__FUNC_DECL_FUNC_DECL_LIST]={
		SYN_EL_NTERM(NTERM_FUNC_DECL_LIST)
		SYN_EL_NTERM(NTERM_FUNC_DECL)
	},
	[RULE_FUNC_DECL_LIST__EPSILON]={
		SYN_EL_TERM(LEX_EOF)
	},
	[RULE_FUNC_DECL]={
		SYN_EL_NTERM(NTERM_OPT_FUNC_BODY)
		SYN_EL_TERM(LEX_RPARENTHESIS)
		SYN_EL_NTERM(NTERM_FUNC_DECL_ARG_LIST)
		SYN_EL_TERM(LEX_LPARENTHESIS)
		SYN_EL_TERM(LEX_ID)
		SYN_EL_NTERM(NTERM_TYPE)
	},
	[RULE_FUNC_DECL_ARG_LIST__DECL_ARG_CONT]={
		SYN_EL_NTERM(NTERM_FUNC_DECL_ARG_LIST_CONT)
		SYN_EL_TERM(LEX_ID)
		SYN_EL_NTERM(NTERM_TYPE)
	},
	[RULE_FUNC_DECL_ARG_LIST__EPSILON]={SYN_EL_NONE},
	[RULE_FUNC_DECL_ARG_LIST_CONT__COMMA_DECL_ARG_CONT]={
		SYN_EL_NTERM(NTERM_FUNC_DECL_ARG_LIST_CONT)
		SYN_EL_TERM(LEX_ID)
		SYN_EL_NTERM(NTERM_TYPE)
		SYN_EL_TERM(LEX_COMMA)
	},
	[RULE_FUNC_DECL_ARG_LIST_CONT__EPSILON]={SYN_EL_NONE},
	[RULE_OPT_FUNC_BODY__SEMICOLON]={
		SYN_EL_TERM(LEX_SEMICOLON)
	},
	[RULE_OPT_FUNC_BODY__COMPOSITE_STATEMENT]={
		SYN_EL_NTERM(NTERM_COMPOSITE_STATEMENT)
	},
	[RULE_STATEMENT__COMPOSITE_STATEMENT]={
		SYN_EL_NTERM(NTERM_COMPOSITE_STATEMENT)
	},
	[RULE_STATEMENT__IF_STATEMENT]={
		SYN_EL_NTERM(NTERM_IF_STATEMENT)
	},
	[RULE_STATEMENT__ASSIGNMENT]={
		SYN_EL_TERM(LEX_SEMICOLON)
		SYN_EL_NTERM(NTERM_ASSIGNMENT)
	},
	[RULE_STATEMENT__FOR_STATEMENT]={
		SYN_EL_NTERM(NTERM_FOR_STATEMENT)
	},
	[RULE_STATEMENT__RETURN_STATEMENT]={
		SYN_EL_TERM(LEX_SEMICOLON)
		SYN_EL_NTERM(NTERM_RETURN_STATEMENT)
	},
	[RULE_STATEMENT__INPUT]={
		SYN_EL_TERM(LEX_SEMICOLON)
		SYN_EL_NTERM(NTERM_INPUT)
	},
	[RULE_STATEMENT__OUTPUT]={
		SYN_EL_TERM(LEX_SEMICOLON)
		SYN_EL_NTERM(NTERM_OUTPUT)
	},
	[RULE_STATEMENT__VAR_DEF]={
		SYN_EL_TERM(LEX_SEMICOLON)
		SYN_EL_NTERM(NTERM_VAR_DEF)
	},
	[RULE_STATEMENT__WHILE_STATEMENT]={
		SYN_EL_NTERM(NTERM_WHILE_STATEMENT)
	},
	[RULE_STATEMENT__DO_WHILE]={
		SYN_EL_TERM(LEX_SEMICOLON)
		SYN_EL_NTERM(NTERM_DO_WHILE)
	},
	[RULE_COMPOSITE_STATEMENT]={
		SYN_EL_TERM(LEX_RBRACE)
		SYN_EL_NTERM(NTERM_STATEMENT_LIST)
		SYN_EL_TERM(LEX_LBRACE)
	},
	[RULE_IF_STATEMENT]={
		SYN_EL_NTERM(NTERM_ELSE_CLAUSE)
		SYN_EL_NTERM(NTERM_STATEMENT)
		SYN_EL_TERM(LEX_RPARENTHESIS)
		SYN_EL_NTERM(NTERM_EXPR_END)
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_LPARENTHESIS)
		SYN_EL_TERM(LEX_IIF)
	},
	[RULE_ELSE_CLAUSE__ELSE_STATEMENT]={
		SYN_EL_NTERM(NTERM_STATEMENT)
		SYN_EL_TERM(LEX_IELSE)
	},
	[RULE_ELSE_CLAUSE__EPSILON]={SYN_EL_NONE},
	[RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST]={
		SYN_EL_NTERM(NTERM_STATEMENT_LIST)
		SYN_EL_NTERM(NTERM_STATEMENT)
	},
	[RULE_STATEMENT_LIST__EPSILON]={SYN_EL_NONE},
	[RULE_ASSIGNMENT]={
		SYN_EL_NTERM(NTERM_EXPR_END)
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_ASSIGN)
		SYN_EL_TERM(LEX_ID)
	},
	[RULE_FOR_STATEMENT]={
		SYN_EL_NTERM(NTERM_STATEMENT)
		SYN_EL_TERM(LEX_RPARENTHESIS)
		SYN_EL_NTERM(NTERM_ASSIGNMENT)
		SYN_EL_TERM(LEX_SEMICOLON)
		SYN_EL_NTERM(NTERM_EXPR_END)
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_SEMICOLON)
		SYN_EL_NTERM(NTERM_VAR_DEF)
		SYN_EL_TERM(LEX_LPARENTHESIS)
		SYN_EL_TERM(LEX_IFOR)
	},
	[RULE_RETURN_STATEMENT]={
		SYN_EL_NTERM(NTERM_EXPR_END)
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_IRETURN)
	},
	[RULE_INPUT]={
		SYN_EL_NTERM(NTERM_INPUT_CHAIN)
		SYN_EL_TERM(LEX_ID)
		SYN_EL_TERM(LEX_STREAM_EXTRACT)
		SYN_EL_TERM(LEX_ICIN)
	},
	[RULE_INPUT_CHAIN__ID_INPUT_CHAIN]={
		SYN_EL_NTERM(NTERM_INPUT_CHAIN)
		SYN_EL_TERM(LEX_ID)
		SYN_EL_TERM(LEX_STREAM_EXTRACT)
	},
	[RULE_INPUT_CHAIN__EPSILON]={SYN_EL_NONE},
	[RULE_OUTPUT]={
		SYN_EL_NTERM(NTERM_OUTPUT_CHAIN)
		SYN_EL_NTERM(NTERM_TERM)
		SYN_EL_TERM(LEX_STREAM_INSERT)
		SYN_EL_TERM(LEX_ICOUT)
	},
	[RULE_OUTPUT_CHAIN__TERM_OUTPUT_CHAIN]={
		SYN_EL_NTERM(NTERM_OUTPUT_CHAIN)
		SYN_EL_NTERM(NTERM_TERM)
		SYN_EL_TERM(LEX_STREAM_INSERT)
	},
	[RULE_OUTPUT_CHAIN__EPSILON]={SYN_EL_NONE},
	[RULE_TERM__ID]={
		SYN_EL_TERM(LEX_ID)
	},
	[RULE_TERM__NUMBER]={
		SYN_EL_TERM(LEX_NUMBER)
	},
	[RULE_TERM__REAL]={
		SYN_EL_TERM(LEX_REAL_NUMBER)
	},
	[RULE_TERM__STRING]={
		SYN_EL_TERM(LEX_STRING_LITERAL)
	},
	[RULE_VAR_DEF__TYPE_ID_OPT_VAR_INIT]={
		SYN_EL_NTERM(NTERM_OPT_VAR_INIT)
		SYN_EL_TERM(LEX_ID)
		SYN_EL_NTERM(NTERM_TYPE)
	},
	[RULE_VAR_DEF__AUTO_ID_VAR_INIT]={
		SYN_EL_NTERM(NTERM_OPT_VAR_INIT)
		SYN_EL_TERM(LEX_ID)
		SYN_EL_TERM(LEX_IAUTO)
	},
	[RULE_OPT_VAR_INIT__VAR_INIT]={
		SYN_EL_NTERM(NTERM_VAR_INIT)
	},
	[RULE_OPT_VAR_INIT__EPSILON]={SYN_EL_NONE},
	[RULE_VAR_INIT__ASSIGN_EXPR]={
		SYN_EL_NTERM(NTERM_EXPR_END)
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_ASSIGN)
	},
	[RULE_WHILE_STATEMENT]={
		SYN_EL_NTERM(NTERM_STATEMENT)
		SYN_EL_NTERM(NTERM_WHILE_CLAUSE)
	},
	[RULE_DO_WHILE]={
		SYN_EL_NTERM(NTERM_WHILE_CLAUSE)
		SYN_EL_NTERM(NTERM_STATEMENT)
		SYN_EL_TERM(LEX_IDO)
	},
	[RULE_WHILE_CLAUSE]={
		SYN_EL_TERM(LEX_RPARENTHESIS)
		SYN_EL_NTERM(NTERM_EXPR_END)
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_LPARENTHESIS)
		SYN_EL_TERM(LEX_IWHILE)
	},
	[RULE_EXPR_END__EPSILON]={SYN_EL_NONE},
	[RULE_TYPE__INT]={
		SYN_EL_TERM(LEX_IINT)
	},
	[RULE_TYPE__DOUBLE]={
		SYN_EL_TERM(LEX_IDOUBLE)
	},
	[RULE_TYPE__STRING]={
		SYN_EL_TERM(LEX_ISTRING)
	},
	/*_*_*_FROM HERE, RIGHT SIDES ARE NO LONGER REVERSED_*_*_*/
	[RULE_EXPR__PARENTHESES_EXPR]={
		SYN_EL_TERM(LEX_LPARENTHESIS)
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_RPARENTHESIS)
	},
	[RULE_EXPR__LEX_ID]={
		SYN_EL_TERM(LEX_ID)
	},
	[RULE_EXPR__LEX_NUMBER]={
		SYN_EL_TERM(LEX_NUMBER)
	},
	[RULE_EXPR__LEX_REAL_NUMBER]={
		SYN_EL_TERM(LEX_REAL_NUMBER)
	},
	[RULE_EXPR__LEX_STRING_LITERAL]={
		SYN_EL_TERM(LEX_STRING_LITERAL)
	},
	[RULE_EXPR__ADD]={
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_PLUS)
		SYN_EL_NTERM(NTERM_EXPR)
	},
	[RULE_EXPR__SUB]={
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_MINUS)
		SYN_EL_NTERM(NTERM_EXPR)
	},
	[RULE_EXPR__MULT]={
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_MULTIPLY)
		SYN_EL_NTERM(NTERM_EXPR)
	},
	[RULE_EXPR__DIV]={
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_DIVIDE)
		SYN_EL_NTERM(NTERM_EXPR)
	},
	[RULE_EXPR__GREATER]={
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_GREATER)
		SYN_EL_NTERM(NTERM_EXPR)
	},
	[RULE_EXPR__LESS]={
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_LESS)
		SYN_EL_NTERM(NTERM_EXPR)
	},
	[RULE_EXPR__GREATEREQ]={
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_GREATEREQ)
		SYN_EL_NTERM(NTERM_EXPR)
	},
	[RULE_EXPR__LESSEQ]={
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_LESSEQ)
		SYN_EL_NTERM(NTERM_EXPR)
	},
	[RULE_EXPR__EQ]={
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_EQUAL)
		SYN_EL_NTERM(NTERM_EXPR)
	},
	[RULE_EXPR__NEQ]={
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_NOTEQUAL)
		SYN_EL_NTERM(NTERM_EXPR)
	},
	[RULE_EXPR__FUNC_NO_PARAM]={
		SYN_EL_TERM(LEX_ID)
		SYN_EL_TERM(LEX_LPARENTHESIS)
		SYN_EL_TERM(LEX_RPARENTHESIS)
	},
	[RULE_EXPR__FUNC_ONE_PARAM]={
		SYN_EL_TERM(LEX_ID)
		SYN_EL_TERM(LEX_LPARENTHESIS)
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_RPARENTHESIS)
	},
	[RULE_EXPR__FUNC_PARAM_LIST]={
		SYN_EL_TERM(LEX_ID)
		SYN_EL_TERM(LEX_LPARENTHESIS)
		SYN_EL_NTERM(NTERM_FUNC_CALL_ARG_LIST)
		SYN_EL_TERM(LEX_RPARENTHESIS)
	},
	[RULE_FUNC_CALL_ARG_LIST__EXPR_EXPR]={
		SYN_EL_NTERM(NTERM_EXPR)
		SYN_EL_TERM(LEX_COMMA)
		SYN_EL_NTERM(NTERM_EXPR)
	},
	[RULE_FUNC_CALL_ARG_LIST__FUNC_CALL_ARG_LIST_EXPR]={
		SYN_EL_NTERM(NTERM_FUNC_CALL_ARG_LIST)
		SYN_EL_TERM(LEX_COMMA)
		SYN_EL_NTERM(NTERM_EXPR)
	},
};

const Rule LL_Table[NTERM_COUNT][LEXEME_TYPE_COUNT]={
	[NTERM_FUNC_DECL_LIST]={
		[LEX_ISTRING]=RULE_FUNC_DECL_LIST__FUNC_DECL_FUNC_DECL_LIST,
		[LEX_IINT]=RULE_FUNC_DECL_LIST__FUNC_DECL_FUNC_DECL_LIST,
		[LEX_IDOUBLE]=RULE_FUNC_DECL_LIST__FUNC_DECL_FUNC_DECL_LIST,
		[LEX_EOF]=RULE_FUNC_DECL_LIST__EPSILON
	},
	[NTERM_FUNC_DECL]={
		[LEX_ISTRING]=RULE_FUNC_DECL,
		[LEX_IINT]=RULE_FUNC_DECL,
		[LEX_IDOUBLE]=RULE_FUNC_DECL,
	},
	[NTERM_FUNC_DECL_ARG_LIST]={
		[LEX_ISTRING]=RULE_FUNC_DECL_ARG_LIST__DECL_ARG_CONT,
		[LEX_IINT]=RULE_FUNC_DECL_ARG_LIST__DECL_ARG_CONT,
		[LEX_IDOUBLE]=RULE_FUNC_DECL_ARG_LIST__DECL_ARG_CONT,
		[LEX_RPARENTHESIS]=RULE_FUNC_DECL_ARG_LIST__EPSILON
	},
	[NTERM_FUNC_DECL_ARG_LIST_CONT]={
		[LEX_COMMA]=RULE_FUNC_DECL_ARG_LIST_CONT__COMMA_DECL_ARG_CONT,
		[LEX_RPARENTHESIS]=RULE_FUNC_DECL_ARG_LIST_CONT__EPSILON,
	},
	[NTERM_OPT_FUNC_BODY]={
		[LEX_SEMICOLON]=RULE_OPT_FUNC_BODY__SEMICOLON,
		[LEX_LBRACE]=RULE_OPT_FUNC_BODY__COMPOSITE_STATEMENT
	},
	[NTERM_STATEMENT]={
		[LEX_LBRACE]=RULE_STATEMENT__COMPOSITE_STATEMENT,
		[LEX_IIF]=RULE_STATEMENT__IF_STATEMENT,
		[LEX_ID]=RULE_STATEMENT__ASSIGNMENT,
		[LEX_IFOR]=RULE_STATEMENT__FOR_STATEMENT,
		[LEX_IRETURN]=RULE_STATEMENT__RETURN_STATEMENT,
		[LEX_ICIN]=RULE_STATEMENT__INPUT,
		[LEX_ICOUT]=RULE_STATEMENT__OUTPUT,
		[LEX_IWHILE]=RULE_STATEMENT__WHILE_STATEMENT,
		[LEX_IDO]=RULE_STATEMENT__DO_WHILE,
		[LEX_ISTRING]=RULE_STATEMENT__VAR_DEF,
		[LEX_IINT]=RULE_STATEMENT__VAR_DEF,
		[LEX_IDOUBLE]=RULE_STATEMENT__VAR_DEF,
		[LEX_IAUTO]=RULE_STATEMENT__VAR_DEF
	},
	[NTERM_COMPOSITE_STATEMENT]={
		[LEX_LBRACE]=RULE_COMPOSITE_STATEMENT
	},
	[NTERM_IF_STATEMENT]={
		[LEX_IIF]=RULE_IF_STATEMENT
	},
	[NTERM_ELSE_CLAUSE]={
		[LEX_IELSE]=RULE_ELSE_CLAUSE__ELSE_STATEMENT,
		[LEX_LBRACE]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_IIF]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_ID]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_IFOR]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_IRETURN]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_ICIN]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_ICOUT]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_IWHILE]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_IDO]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_ISTRING]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_IINT]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_IDOUBLE]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_IAUTO]=RULE_ELSE_CLAUSE__EPSILON,
		[LEX_RBRACE]=RULE_ELSE_CLAUSE__EPSILON
	},
	[NTERM_STATEMENT_LIST]={
		[LEX_LBRACE]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_IIF]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_ID]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_IFOR]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_IRETURN]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_ICIN]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_ICOUT]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_IWHILE]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_IDO]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_ISTRING]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_IINT]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_IDOUBLE]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_IAUTO]=RULE_STATEMENT_LIST__STATEMENT_STATEMENT_LIST,
		[LEX_RBRACE]=RULE_STATEMENT_LIST__EPSILON,
	},
	[NTERM_ASSIGNMENT]={
		[LEX_ID]=RULE_ASSIGNMENT
	},
	[NTERM_FOR_STATEMENT]={
		[LEX_IFOR]=RULE_FOR_STATEMENT
	},
	[NTERM_RETURN_STATEMENT]={
		[LEX_IRETURN]=RULE_RETURN_STATEMENT
	},
	[NTERM_INPUT]={
		[LEX_ICIN]=RULE_INPUT
	},
	[NTERM_INPUT_CHAIN]={
		[LEX_STREAM_EXTRACT]=RULE_INPUT_CHAIN__ID_INPUT_CHAIN,
		[LEX_SEMICOLON]=RULE_INPUT_CHAIN__EPSILON
	},
	[NTERM_OUTPUT]={
		[LEX_ICOUT]=RULE_OUTPUT
	},
	[NTERM_OUTPUT_CHAIN]={
		[LEX_STREAM_INSERT]=RULE_OUTPUT_CHAIN__TERM_OUTPUT_CHAIN,
		[LEX_SEMICOLON]=RULE_OUTPUT_CHAIN__EPSILON
	},
	[NTERM_WHILE_STATEMENT]={
		[LEX_IWHILE]=RULE_WHILE_STATEMENT
	},
	[NTERM_DO_WHILE]={
		[LEX_IDO]=RULE_DO_WHILE
	},
	[NTERM_WHILE_CLAUSE]={
		[LEX_IWHILE]=RULE_WHILE_CLAUSE
	},
	[NTERM_TERM]={
		[LEX_ID]=RULE_TERM__ID,
		[LEX_NUMBER]=RULE_TERM__NUMBER,
		[LEX_REAL_NUMBER]=RULE_TERM__REAL,
		[LEX_STRING_LITERAL]=RULE_TERM__STRING
	},
	[NTERM_VAR_DEF]={
		[LEX_ISTRING]=RULE_VAR_DEF__TYPE_ID_OPT_VAR_INIT,
		[LEX_IINT]=RULE_VAR_DEF__TYPE_ID_OPT_VAR_INIT,
		[LEX_IDOUBLE]=RULE_VAR_DEF__TYPE_ID_OPT_VAR_INIT,
		[LEX_IAUTO]=RULE_VAR_DEF__AUTO_ID_VAR_INIT
	},
	[NTERM_OPT_VAR_INIT]={
		[LEX_ASSIGN]=RULE_OPT_VAR_INIT__VAR_INIT,
		[LEX_SEMICOLON]=RULE_OPT_VAR_INIT__EPSILON
	},
	[NTERM_VAR_INIT]={
		[LEX_ASSIGN]=RULE_VAR_INIT__ASSIGN_EXPR
	},
	[NTERM_EXPR]={
		RULE_ERROR//expr will be handled by a different parser
	},
	[NTERM_EXPR_END]={
		[LEX_STREAM_INSERT]=RULE_EXPR_END__EPSILON,
		[LEX_SEMICOLON]=RULE_EXPR_END__EPSILON,
		[LEX_RPARENTHESIS]=RULE_EXPR_END__EPSILON
	},
	[NTERM_FUNC_CALL_ARG_LIST]={
		RULE_ERROR//can only occur inside expr
	},
	[NTERM_TYPE]={
		[LEX_ISTRING]=RULE_TYPE__STRING,
		[LEX_IINT]=RULE_TYPE__INT,
		[LEX_IDOUBLE]=RULE_TYPE__DOUBLE,
	},
};

typedef enum{
	PREC_EMPTY,
	PREC_LOWER,
	PREC_EQUAL,
	PREC_HIGHER
}Precedence;

typedef enum{
	OP_ADDSUB,
	OP_MULDIV,
	OP_RELAT,
	OP_ID,
	OP_VAL,
	OP_LPAREN,
	OP_RPAREN,
	OP_COMMA,
	OP_END,
	OPERATOR_COUNT
}OperatorType;

Precedence precedenceTable[OPERATOR_COUNT][OPERATOR_COUNT]={
	[OP_ADDSUB]={
		[OP_ADDSUB]=PREC_HIGHER,
		[OP_MULDIV]=PREC_LOWER,
		[OP_RELAT]=PREC_HIGHER,
		[OP_ID]=PREC_LOWER,
		[OP_VAL]=PREC_LOWER,
		[OP_LPAREN]=PREC_LOWER,
		[OP_RPAREN]=PREC_HIGHER,
		[OP_COMMA]=PREC_HIGHER,
		[OP_END]=PREC_HIGHER,
	},
	[OP_MULDIV]={
		[OP_ADDSUB]=PREC_HIGHER,
		[OP_MULDIV]=PREC_HIGHER,
		[OP_RELAT]=PREC_HIGHER,
		[OP_ID]=PREC_LOWER,
		[OP_VAL]=PREC_LOWER,
		[OP_LPAREN]=PREC_LOWER,
		[OP_RPAREN]=PREC_HIGHER,
		[OP_COMMA]=PREC_HIGHER,
		[OP_END]=PREC_HIGHER,
	},
	[OP_RELAT]={
		[OP_ADDSUB]=PREC_LOWER,
		[OP_MULDIV]=PREC_LOWER,
		[OP_RELAT]=PREC_HIGHER,
		[OP_ID]=PREC_LOWER,
		[OP_VAL]=PREC_LOWER,
		[OP_LPAREN]=PREC_LOWER,
		[OP_RPAREN]=PREC_HIGHER,
		[OP_COMMA]=PREC_HIGHER,
		[OP_END]=PREC_HIGHER,
	},
	[OP_ID]={
		[OP_ADDSUB]=PREC_HIGHER,
		[OP_MULDIV]=PREC_HIGHER,
		[OP_RELAT]=PREC_HIGHER,
		[OP_LPAREN]=PREC_EQUAL,
		[OP_RPAREN]=PREC_HIGHER,
		[OP_COMMA]=PREC_HIGHER,
		[OP_END]=PREC_HIGHER
	},
	[OP_VAL]={
		[OP_ADDSUB]=PREC_HIGHER,
		[OP_MULDIV]=PREC_HIGHER,
		[OP_RELAT]=PREC_HIGHER,
		[OP_RPAREN]=PREC_HIGHER,
		[OP_COMMA]=PREC_HIGHER,
		[OP_END]=PREC_HIGHER
	},
	[OP_LPAREN]={
		[OP_ADDSUB]=PREC_LOWER,
		[OP_MULDIV]=PREC_LOWER,
		[OP_RELAT]=PREC_LOWER,
		[OP_ID]=PREC_LOWER,
		[OP_VAL]=PREC_LOWER,
		[OP_LPAREN]=PREC_LOWER,
		[OP_RPAREN]=PREC_EQUAL,
		[OP_COMMA]=PREC_LOWER,
	},
	[OP_RPAREN]={
		[OP_ADDSUB]=PREC_HIGHER,
		[OP_MULDIV]=PREC_HIGHER,
		[OP_RELAT]=PREC_HIGHER,
		[OP_RPAREN]=PREC_HIGHER,
		[OP_END]=PREC_HIGHER,
	},
	[OP_COMMA]={
		[OP_ADDSUB]=PREC_LOWER,
		[OP_MULDIV]=PREC_LOWER,
		[OP_RELAT]=PREC_LOWER,
		[OP_ID]=PREC_LOWER,
		[OP_VAL]=PREC_LOWER,
		[OP_LPAREN]=PREC_LOWER,
		[OP_RPAREN]=PREC_HIGHER,
		[OP_COMMA]=PREC_HIGHER,
	},
	[OP_END]={
		[OP_ADDSUB]=PREC_LOWER,
		[OP_MULDIV]=PREC_LOWER,
		[OP_RELAT]=PREC_LOWER,
		[OP_ID]=PREC_LOWER,
		[OP_VAL]=PREC_LOWER,
		[OP_LPAREN]=PREC_LOWER,
	}

};

OperatorType translate_tok_type(TokenType tokType){
	switch(tokType){
	case LEX_PLUS:
	case LEX_MINUS:
		return OP_ADDSUB;
	case LEX_COMMA:
		return OP_COMMA;
	case LEX_GREATER:
	case LEX_GREATEREQ:
	case LEX_LESS:
	case LEX_LESSEQ:
	case LEX_NOTEQUAL:
	case LEX_EQUAL:
		return OP_RELAT;
	case LEX_NUMBER:
	case LEX_REAL_NUMBER:
	case LEX_STRING_LITERAL:
		return OP_VAL;
	case LEX_ID:
		return OP_ID;
	case LEX_LPARENTHESIS:
		return OP_LPAREN;
	case LEX_RPARENTHESIS:
		return OP_RPAREN;
	case LEX_MULTIPLY:
	case LEX_DIVIDE:
		return OP_MULDIV;
	default:
		return OP_END;
	}
}

bool syntactic_elements_are_equal(SyntacticElement one, SyntacticElement other){
	switch(one.type){
	case SYNT_NONTERMINAL:
		return other.type==SYNT_NONTERMINAL&&other.nonterminal==one.nonterminal;
	case SYNT_TERMINAL:
		return other.type==SYNT_TERMINAL&&other.terminal==one.terminal;
	default:
		return one.type==other.type;
	}
}

#define set_real_retval(to) do{real_retval=real_retval==RET_EOK?to:real_retval;}while(0)

int get_next_valid_token(FILE *progfile, Token *currentToken){
	int retval, real_retval=RET_EOK;
	do{
		retval=get_token(progfile, currentToken);
		//get_token should print its own error, so no printing here
		if(retval==RET_INTERN){
			Throw(RET_INTERN);
		}
		set_real_retval(retval);
	}while(retval!=RET_EOK);
	return real_retval;
}

int analyze_expr_syntax(FILE *progfile, ParseTree *output, Token *currentToken){
	int retval=RET_EOK;
	int real_retval=RET_EOK;
	Vector_SyntacticElement exprStack;
	Vector_Token valStack;
	if((retval=vector_Token_init(&valStack))!=RET_EOK){
		Throw(retval);
	}
	if((retval=vector_SyntacticElement_init(&exprStack))!=RET_EOK){
		vector_Token_destroy(&valStack);
		Throw(retval);
	}
	Try{
		vector_SyntacticElement_push_back(&exprStack, (SyntacticElement){.type=SYNT_EXPR_START});
		OperatorType next_op=translate_tok_type(currentToken->type);
		if(next_op==OP_VAL||next_op==OP_ID){
			//if the token holds a value, push it on the value stack
			vector_Token_push_back(&valStack, *currentToken);
		}
		while(real_retval==RET_EOK){
			OperatorType top_op;
			size_t top_term_index;
			{
				//get the top nonterminal and transform it into OperatorType
				SyntacticElement top_term;
				top_term_index=vector_SyntacticElement_count(&exprStack);
				do{
					assert(top_term_index>0);
					top_term_index--;
					top_term=*vector_SyntacticElement_at(&exprStack, top_term_index);
				}while(top_term.type!=SYNT_TERMINAL&&top_term.type!=SYNT_EXPR_START);
				if(top_term.type==SYNT_EXPR_START){
					top_op=OP_END;
				}
				else{
					top_op=translate_tok_type(top_term.terminal);
					assert(top_op!=OP_END);
				}
			}
			switch(precedenceTable[top_op][next_op]){
			case PREC_LOWER:
				vector_SyntacticElement_insert_at(&exprStack, top_term_index+1, (SyntacticElement){.type=SYNT_HANDLE_START});
				//note that there is no break; here. This is intentional.
			case PREC_EQUAL:
				vector_SyntacticElement_push_back(&exprStack, (SyntacticElement){.type=SYNT_TERMINAL, .terminal=currentToken->type});
				if((retval=get_next_valid_token(progfile, currentToken))!=RET_EOK){
					//get_token should print its own error
					set_real_retval(retval);
				}
				else{
					next_op=translate_tok_type(currentToken->type);
					if(next_op==OP_VAL||next_op==OP_ID){
						//if the token holds a value, push it on the value stack
						vector_Token_push_back(&valStack, *currentToken);
					}
				}
				break;
			case PREC_HIGHER:{
				size_t exprStartIndex=vector_SyntacticElement_count(&exprStack)-1;
				//push a stopper
				vector_SyntacticElement_push_back(&exprStack, (SyntacticElement){.type=SYNT_NONE});
				//the top element will never be the handle start delimiter, but if the top element
				//holds a value, we must send it to the output.
				SyntacticElement currentElement;
				while((currentElement=*vector_SyntacticElement_at(&exprStack, exprStartIndex)).type!=SYNT_HANDLE_START){
					//if we are reducing token with a value, the value must be output with the rule.
					if(currentElement.type==SYNT_TERMINAL){
						switch(currentElement.terminal){
						case LEX_ID:
						case LEX_STRING_LITERAL:
						case LEX_NUMBER:
						case LEX_REAL_NUMBER:
							vector_Token_push_back(&(output->values), vector_Token_pop_back(&valStack));
						default:
							;
						}
					}
					exprStartIndex--;
				}
				//we want the index of the actual handle start, not the start delimiter
				exprStartIndex++;
				//now we have the start of expression
				//so iterate over rules and check if any of the handles fit.
				Rule rule;
				for(rule=RULES_EXPRESSIONS_START+1; rule<RULES_END; rule++){
					//we start at StartIndex+1 because at StartIndex is the handle start delimiter
					SyntacticElement elem;
					for(size_t i=0; i<=MAX_HANDLE_LENGTH; i++){
						elem=*vector_SyntacticElement_at(&exprStack, i+exprStartIndex);
						//if they don't match, try next rule
						if(!syntactic_elements_are_equal(rules[rule][i], elem)){
							break;
						}
						//if we are at the end of the pattern, we found the rule
						if(elem.type==SYNT_NONE){
							goto rule_found;
						}
					}
					//if the inner for ends , we have found a rule
				}
				//if no rule matched
				set_real_retval(RET_SYN);
				error_message("Unexpected token %t in expression", currentToken->line);
				break;
			rule_found:
				//if rule was found, pop the handle off the stack
				vector_Rule_push_back(&output->rules, rule);
				while(vector_SyntacticElement_pop_back(&exprStack).type!=SYNT_HANDLE_START)
					;
				//and replace it by the appropriate nonterminal
				SyntacticElement to_push={.type=SYNT_NONTERMINAL};
				if(rule>RULES_ARGUMENTS_START){
					to_push.nonterminal=NTERM_FUNC_CALL_ARG_LIST;
				}
				else{
					to_push.nonterminal=NTERM_EXPR;
				}
				vector_SyntacticElement_push_back(&exprStack, to_push);
				break;
			}
			case PREC_EMPTY:
				//check that the top of the stack is $[expr]
				if(vector_SyntacticElement_count(&exprStack)==2){
					if(vector_SyntacticElement_at(&exprStack, 1)->type==SYNT_NONTERMINAL
					&&vector_SyntacticElement_at(&exprStack, 1)->nonterminal==NTERM_EXPR){
						assert(vector_SyntacticElement_at(&exprStack, 0)->type==SYNT_EXPR_START);
						//break out of the while
						goto end_success;
					}
				}
				//if it isn't, set retval;
				//TODO: error
				set_real_retval(RET_SYN);
				break;
			}
		}
	end_success:
		;
	}
	Catch(retval){
		//clean up and rethrow.
		vector_SyntacticElement_destroy(&exprStack);
		vector_Token_destroy(&valStack);
		Throw(retval);
	}
	vector_SyntacticElement_destroy(&exprStack);
	vector_Token_destroy(&valStack);
	return real_retval;
}

void rollback_analysis_stack(Vector_SyntacticElement *synt_stack, size_t new_count){
	for(size_t i=vector_SyntacticElement_count(synt_stack); i>new_count; i--){
		vector_SyntacticElement_pop_back(synt_stack);
	}
}

void get_next_expected_token(FILE *progfile, Vector_SyntacticElement *currentAnalysisState, Token *currentToken){
	//first, pop off all the nonterminals to find the next expected terminal.
	SyntacticElement recovery_attempt;
	while(true){
		//will always end, as EOF will eventually be encountered and 
		//EOF is in the LL table entry of [func_decl_list], which is 
		//always at the bottom of the stack.
		get_next_valid_token(progfile, currentToken);
		size_t i=vector_SyntacticElement_count(currentAnalysisState)-1;
		while(i!=SIZE_MAX){
			recovery_attempt=*vector_SyntacticElement_at(currentAnalysisState, i);
			if(recovery_attempt.type==SYNT_NONTERMINAL){
				if(LL_Table[recovery_attempt.nonterminal][currentToken->type]!=RULE_ERROR){
					rollback_analysis_stack(currentAnalysisState, i+1);
					return;
				}
			}
			i--;
		}
	}
}

int analyze_program_syntax(FILE *progfile, ParseTree *output){
	Vector_SyntacticElement vectorVar;
	Vector_SyntacticElement *stack=&vectorVar;
	int retval;
	int real_retval=RET_EOK;
	if((retval=vector_SyntacticElement_init(stack))!=RET_EOK){
		return retval;
	}
	CEXCEPTION_T exception;
	Try{
		//initialize the stack with the starting nonterminal
		vector_SyntacticElement_push_back(stack, (SyntacticElement){.type=SYNT_NONTERMINAL, .nonterminal=NTERM_FUNC_DECL_LIST});
		Token currentToken;
		//get the first token
		retval=get_next_valid_token(progfile, &currentToken);
		set_real_retval(retval);
		while(vector_SyntacticElement_count(stack)){
			SyntacticElement topElement=vector_SyntacticElement_pop_back(stack);
			if(topElement.type==SYNT_TERMINAL){
				if(topElement.terminal==currentToken.type){
					switch(currentToken.type){
					case LEX_ID:
					case LEX_STRING_LITERAL:
					case LEX_NUMBER:
					case LEX_REAL_NUMBER:
						//if the token holds a value, it must be passed to semantic analysis.
						vector_Token_push_back(&(output->values), currentToken);
					default:
						break;
					}
				}
				else{
					error_message("Unexpected token %t in main syntactic analysis", currentToken.line, currentToken);
					set_real_retval(RET_SYN);
					get_next_expected_token(progfile, stack, &currentToken);
					//we already have the needed token, so next iteration.
					continue;
				}
				retval=get_next_valid_token(progfile, &currentToken);
				set_real_retval(retval);
			}
			else{
				if(topElement.nonterminal==NTERM_EXPR){
					//use the precedence parser
					retval=analyze_expr_syntax(progfile, output, &currentToken);
					set_real_retval(retval);
					if(retval!=RET_EOK){
						get_next_expected_token(progfile, stack, &currentToken);
					}
				}
				else{
					Rule ruleToUse=LL_Table[topElement.nonterminal][currentToken.type];
					if(ruleToUse==RULE_ERROR){
						error_message("unexpected token %t", currentToken.line, currentToken);
						//push top element back to the stack
						vector_SyntacticElement_push_back(stack, topElement);
						set_real_retval(RET_SYN);
						get_next_expected_token(progfile, stack, &currentToken);
					}
					else{
						vector_Rule_push_back(&output->rules, ruleToUse);
						SyntacticElement currentSyntElement=rules[ruleToUse][0];
						for(int i=1; currentSyntElement.type!=SYNT_NONE; i++){
							vector_SyntacticElement_push_back(stack, currentSyntElement);
							currentSyntElement=rules[ruleToUse][i];
						}
					}
				}
			}
		}
	}
	Catch(exception){
		if(exception==RET_INTERN){
			//TODO: print an out of memory error?
			set_real_retval(RET_INTERN);
		}
	}
	vector_SyntacticElement_destroy(stack);
	return real_retval;
}

#undef set_real_retval
