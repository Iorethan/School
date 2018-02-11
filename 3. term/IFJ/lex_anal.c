/*
 * lex_anal.c
 * Autori: Radek Vit, Nikola Valesova
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


#include "lex_anal.h"

/* appends ch to array and checks for error */
#define append(ch) do {\
		if(array_append_char(string,(ch)) != EXIT_SUCCESS) \
		{ \
			array_destroy(string); \
			return RET_INTERN; \
		}\
		} while(0)

#define correct_position() (position == 1) ? line-- : position--;

/* reads a lexem and sends corresponding token */
int get_token(FILE *input, Token *token)
{
	// number of current line
	static unsigned int line = 1;
	// position on line
	static unsigned int position = 1;
	
	Array stringInstance;
	Array *string=&stringInstance; //for storing identificator names

	enum State state = INIT;
	int ch; //character storage

	while (1)	// finite state machine
	{
		ch = getc(input);
		
		/* position tracking */
		if (ch == '\n')
		{
			position = 1;
			line++;
		}
		else
			position++;
		/* end of position tracking */

		/* finite state mashine */
		switch(state)
		{
		case INIT: // starting state
			token->line = line;
			token->position = position;
			
			if (isspace(ch))	// skipping whitespace characters
				state = INIT;
			else if (ch == '/')	// possible start of a comment
				state = COMMENT_MAYBE;
			else // lexems
			{
				if (ch == EOF)
				{
					token->type = LEX_EOF;
					return RET_EOK;
				}
				else if (ch == '+')
				{
					token->type = LEX_PLUS;
					return RET_EOK;
				}
				else if (ch == '-')
				{
					token->type = LEX_MINUS;
					return RET_EOK;
				}
				else if (ch == '*')
				{
					token->type = LEX_MULTIPLY;
					return RET_EOK;
				}
				else if (ch == ',')
				{
					token->type = LEX_COMMA;
					return RET_EOK;
				}
				else if (ch == '(')
				{
					token->type = LEX_LPARENTHESIS;
					return RET_EOK;
				}
				else if (ch == ')')
				{
					token->type = LEX_RPARENTHESIS;
					return RET_EOK;
				}
				else if (ch == '{')
				{
					token->type = LEX_LBRACE;
					return RET_EOK;
				}
				else if (ch == '}')
				{
					token->type = LEX_RBRACE;
					return RET_EOK;
				}
				else if (ch == ';')
				{
					token->type = LEX_SEMICOLON;
					return RET_EOK;
				}
				else if (ch == '<')
					state = LESS;
				else if (ch == '>')
					state = GREATER;
				else if (ch == '=')
					state = EQUAL;
				else if (ch == '!')
					state = N_EQUAL;
				else
				{
					if(array_init(string) != EXIT_SUCCESS)
						return RET_INTERN;
	
					if (isalpha(ch) || ch == '_')
					{
						state = IDENTIFICATOR;
						append(ch);
					}
					else if (isdigit(ch))
					{
						state = NUMBER;
						append(ch);
					}
					else if (ch == '"')
						state = STRING;
					else
					{
						array_destroy(string);
						return RET_LEX;
					}
				}
			}
			break;

		case IDENTIFICATOR:
			if (isalnum(ch) || ch == '_')	// legal characters
				append(ch);
			else //end of ID lexem
			{
				correct_position();
				ungetc(ch,input); //returning trailing character

				if (is_keyword(string->buffer, token)){
					array_destroy(string);
					return RET_EOK;
				}
				token->type = LEX_ID;
				token->IDName = string->buffer;
				return RET_EOK;
			}
			break;

		case NUMBER:
			if (isdigit(ch))
				append(ch);
			else if (ch == '.')
			{
				state = REAL_DOT; 
				append(ch);
			}
			else if (ch == 'E' || ch == 'e')
			{
				state = REAL_EXP_SIGN;
				append(ch);
			}
			else // read number succesfuly
			{
				correct_position();
				ungetc(ch,input);
				token->type = LEX_NUMBER;
				token->number = strtol(string->buffer,NULL,10);
				array_destroy(string);
				return RET_EOK;
			}
			break;

		case REAL_DOT: // dot separating integer and decimal parts found
			if (isdigit(ch))
			{
				state = REAL_DECIMAL;
				append(ch);
			}
			else // invalid lexem
			{
				array_destroy(string);
				return RET_LEX;
			}
			break;

		case REAL_DECIMAL: // decimal part
			if (isdigit(ch))
				append(ch);
			else if (ch == 'e' || ch == 'E')
			{
				append(ch);
				state = REAL_EXP_SIGN;
			}
			else // done reading double
			{
				correct_position();
				ungetc(ch,input);
				token->type = LEX_REAL_NUMBER;
				token->real = strtod(string->buffer, NULL);
				array_destroy(string);
				return RET_EOK;
			}
			break;

		case REAL_EXP_SIGN:	// 'E' or 'e' representing exponent found
			if (ch == '+' || ch == '-')
			{
				append(ch);
				state = REAL_PLUSMINUS;
			}
			else if (isdigit(ch))
			{
				append(ch);
				state = REAL_EXP;
			}

			else // invalid lexem
			{
				array_destroy(string);
				return RET_LEX;
			}
			break;

		case REAL_PLUSMINUS: // + or - found, at least one more digit needed
			if (isdigit(ch))
			{
				state = REAL_EXP;
				append(ch);
			}
			else // invalid lexem
			{
				array_destroy(string);
				return RET_LEX;
			}
			break;

		case REAL_EXP:	// exponent part of a number
			if (isdigit(ch))
				append(ch);
			else // done reading double
			{
				correct_position();
				ungetc(ch,input);
				token->type = LEX_REAL_NUMBER;
				token->real = strtod(string->buffer,NULL);
				array_destroy(string);
				return RET_EOK;
			}
			break;

		case STRING:	// string
			if (ch == '\\')
				state = ESCAPE_SEQUENCE;
			else if (ch == '\n' || ch == EOF)
			{
				array_destroy(string);
				return RET_LEX;
			}
			else if (ch == '"') // done
			{	
				token->string = string->buffer;
				token->type = LEX_STRING_LITERAL;
				return RET_EOK;
			}
			else // continuing with string
				append(ch);
			break;

		case ESCAPE_SEQUENCE:
			if (ch == 'n')
			{
				append('\n');
				state = STRING;
			}
			else if (ch == 't')
			{
				append('\t');
				state = STRING;
			}
			else if (ch == '\\')
			{
				append('\\');
				state = STRING;
			}
			else if (ch == '\"')
			{
				append('\"');
				state = STRING;
			}
			else if (ch == 'x')
				state = STRING_HD1;
			else if (ch == '\n' || ch == EOF)
			{
				array_destroy(string);
				return RET_LEX;
			}
			else
			{
				array_destroy(string);
				state = STRING_ERR; //going to end of string literal
			}
			break;

		case STRING_HD1:
			if (ch >= '0' && ch <= '9')
			{
				ch = (ch - '0') << (CHAR_BIT/2);
				append(ch);
				state = STRING_HD2;
			}
			else if (ch >= 'A' && ch <= 'F')
			{
				ch = (ch - 'A' + 10) << (CHAR_BIT/2); //10 is \x offset for A
				append(ch);
				state = STRING_HD2;
			}
			else if (ch >= 'a' && ch <= 'f')
			{
				ch = (ch - 'a' + 10) << (CHAR_BIT/2); //10 is \x offset for a
				append(ch);
				state = STRING_HD2;
			}
			else if (ch == '\n' || ch == '\"' || ch == EOF)
			{
				array_destroy(string);
				return RET_LEX;
			}
			else
			{
				array_destroy(string);
				state = STRING_ERR; //going to end of string literal
			}
			break;

		case STRING_HD2:
			if (ch >= '0' && ch <= '9')
			{
				array_add_to_last_written(string,(ch - '0'));
				state = STRING;
				/* checking for \x00 */
				if(string->buffer[string->pos - 1] == 0)
				{
					array_destroy(string);
					state = STRING_ERR;
				}
			}
			else if (ch >= 'A' && ch <= 'F')
			{
				array_add_to_last_written(string,(ch - 'A' + 10));
				state = STRING;
			}
			else if (ch >= 'a' && ch <= 'f')
			{
				array_add_to_last_written(string,(ch - 'a' + 10));
				state = STRING;
			}
			else if (ch == '\n' || ch == '"' || ch == EOF)
			{
				array_destroy(string);
				return RET_LEX;
			}
			else
			{
				array_destroy(string);
				state = STRING_ERR; //going to end of string literal
			}
			break;

		case STRING_ERR:
			if (ch == '\n' || ch == '\"' || ch == EOF)
				return RET_LEX;
			else if (ch == '\\')
				state = STRING_ERR_ESC;
			break;

		case STRING_ERR_ESC:
			if (ch == '\n' || ch == EOF) //ending if newline or end of file, else ignoring input
				return RET_LEX;
			else
				state = STRING_ERR;
			break;

		case COMMENT_MAYBE: // possible comment
			if (ch == '*')
				state = COMMENT_BLOCK;
			else if (ch == '/')
				state = COMMENT_LINE;
			else	// '/' as division operator
			{
				correct_position();
				ungetc(ch,input);
				token->type = LEX_DIVIDE;
				return RET_EOK;
			}
			break;

		case COMMENT_LINE: // inside line comment
			if (ch == '\n')
				state = INIT;
			else if (ch == EOF)
			{
				token->type = LEX_EOF;
				return RET_EOK;
			}
			break;

		case COMMENT_BLOCK: // inside block comment
			if (ch == '*')
				state = COMMENT_END_MAYBE;
			else if (ch == EOF)
				return RET_LEX;;
			break;

		case COMMENT_END_MAYBE: // encountered '*' inside block comment
			if (ch == '/')
				state = INIT;
			else if (ch != '*')
				state = COMMENT_BLOCK; 
			else if (ch == EOF)
				return RET_LEX;
			break;

		case LESS: // '<'
			if (ch == '=')
				token->type = LEX_LESSEQ;
			else if (ch == '<')
				token->type = LEX_STREAM_INSERT;
			else
			{
				correct_position();
				ungetc(ch, input);
				token->type = LEX_LESS;
			}
			return RET_EOK;
			break;

		case GREATER: // '>'
			if (ch == '=')
				token->type = LEX_GREATEREQ;
			else if (ch == '>')
				token->type = LEX_STREAM_EXTRACT;
			else
			{
				correct_position();
				ungetc(ch, input);
				token->type = LEX_GREATER;
			}
			return RET_EOK;
			break;

		case EQUAL: // '='
			if (ch == '=')
				token->type = LEX_EQUAL;
			else
			{
				correct_position();
				ungetc(ch, input);
				token->type = LEX_ASSIGN;
			}
			return RET_EOK;
			break;

		case N_EQUAL: // '!'
			if (ch == '=')
			{
				token->type = LEX_NOTEQUAL;
				return RET_EOK;
			}
			else
			{
				correct_position();
				ungetc(ch, input);
				return RET_LEX;
			}
			break;
		}
	}
}

// determines whether a string is a keyword
bool is_keyword(char *string,Token *token)
{
	switch(strlen(string))
	{
		case 2:
			if(!strcmp(string,"if"))
			{
				token->type = LEX_IIF;
				return true;
			}
			else if(!strcmp(string,"do"))
			{
				token->type = LEX_IDO;
				return true;
			}
			break;
		case 3:
			if(!strcmp(string,"int"))
			{
				token->type = LEX_IINT;
				return true;
			}
			if(!strcmp(string,"for"))
			{
				token->type = LEX_IFOR;
				return true;
			}
			if(!strcmp(string,"cin"))
			{
				token->type = LEX_ICIN;
				return true;
			}
			break;
		case 4:
			if(!strcmp(string,"else"))
			{
				token->type = LEX_IELSE;
				return true;
			}
			if(!strcmp(string,"auto"))
			{
				token->type = LEX_IAUTO;
				return true;
			}
			if(!strcmp(string,"cout"))
			{
				token->type = LEX_ICOUT;
				return true;
			}
			break;
		case 5:
			if(!strcmp(string,"while"))
			{
				token->type = LEX_IWHILE;
				return true;
			}
			break;
		case 6:
			if(!strcmp(string,"return"))
			{
				token->type = LEX_IRETURN;
				return true;
			}
			if(!strcmp(string,"double"))
			{
				token->type = LEX_IDOUBLE;
				return true;
			}
			if(!strcmp(string,"string"))
			{
				token->type = LEX_ISTRING;
				return true;
			}
			break;
	}

	return false;
}
