/***************************************************************************
    File                 : parser.y
    Project              : LabPlot
    Description          : Parser for mathematical expressions
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2014-2016 Stefan Gerlach  (stefan.gerlach@uni.kn)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/


%{
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <locale.h>
#include "parser.h"
#include "constants.h"
#include "functions.h"

int yyerror(const char *err);
int yylex();

double res;
%}

%union {
double dval;  /* For returning numbers */
symrec *tptr;   /* For returning symbol-table pointers */
}

%token <dval> NUM 	/* Simple double precision number */
%token <tptr> VAR FNCT	/* Variable and Function */
%type  <dval> expr

%right '='
%left '-' '+'
%left '*' '/'
%left NEG     /* Negation--unary minus */
%right '^'    /* Exponential */

%%
input:   /* empty */
	| input line
;

line:	'\n'
	| expr '\n'   { res=$1; }
	| error '\n' { yyerrok; }
;

expr:      NUM       { $$ = $1;                         }
| VAR                { $$ = $1->value.var;              }
| VAR '=' expr       { $$ = $3; $1->value.var = $3;     }
| FNCT '(' ')'       { $$ = (*($1->value.fnctptr))();   }
| FNCT '(' expr ')'  { $$ = (*($1->value.fnctptr))($3); }
| FNCT '(' expr ',' expr ')'  { $$ = (*($1->value.fnctptr))($3,$5); }
| FNCT '(' expr ',' expr ','expr ')'  { $$ = (*($1->value.fnctptr))($3,$5,$7); }
| FNCT '(' expr ',' expr ',' expr ','expr ')'  { $$ = (*($1->value.fnctptr))($3,$5,$7,$9); }
| expr '+' expr      { $$ = $1 + $3;                    }
| expr '-' expr      { $$ = $1 - $3;                    }
| expr '*' expr      { $$ = $1 * $3;                    }
| expr '/' expr      { $$ = $1 / $3;                    }
| '-' expr  %prec NEG{ $$ = -$2;                        }
| expr '^' expr      { $$ = pow ($1, $3);               }
| expr '*' '*' expr  { $$ = pow ($1, $4);               }
| '(' expr ')'       { $$ = $2;                         }
;

%%

symrec *putsym(const char *, int); /* put symbol in symbol table */
symrec *getsym(const char *);
void init_table(void);		/* initialize symbol table */
void delete_table(void);	/* delete symbol table */
int parse_errors();

unsigned int pos;
#define PARSE_STRING_SIZE 500	/* big enough? */
char string[PARSE_STRING_SIZE];

/* Enable Parser DEBUGGING */
/*
#define PDEBUG
*/

void pdebug(const char *str) {
#ifdef PDEBUG
	printf("%s\n", str);
#else
	(void)str;
#endif
}

/* The symbol table: a chain of `struct symrec'.  */
symrec *sym_table = (symrec *) 0;

double parse(const char *str) {
#ifdef PDEBUG
	printf("\nPARSER: parse(\"%s\")\n", str);
#endif
	pos = 0;

	/* reset string, because it's global! */
	bzero(string, PARSE_STRING_SIZE);

	/* leave space to terminate string by "\n\0" */
	strncpy(string, str, PARSE_STRING_SIZE - 2);
	string[strlen(string)] = '\n';

	/* be sure that the symbol table has been initialized */
	if (!sym_table)
	   init_table();

	yyparse();

/*	pdebug("PARSER: parse() DONE");*/
	return res;
}

int parse_errors() {
	return yynerrs;
}

int yyerror (const char *s) {
	printf ("PARSER ERROR: %s\n", s);
	return 0;
}

/* put arithmetic functions in table. */
void init_table (void) {
	pdebug("PARSER: init_table()");

	symrec *ptr;
	int i;
	/* add functions */
	for (i = 0; _functions[i].name != 0; i++) {
		ptr = putsym (_functions[i].name, FNCT);
		ptr->value.fnctptr = _functions[i].fnct;
	}
	/* add constants */
	for (i = 0; _constants[i].name != 0; i++) {
		ptr = putsym (_constants[i].name, VAR);
		ptr->value.var = _constants[i].value;
	}
	pdebug("PARSER: init_table() DONE");
}

void delete_table(void) {
	symrec *tmp;
	while(sym_table) {
		tmp = sym_table;
		sym_table = sym_table->next;
		free(tmp->name);
		free(tmp);
	}
}

symrec* putsym (const char *sym_name, int sym_type) {
#ifdef PDEBUG
	printf("PARSER: putsym(): sym_name = %s\n", sym_name);
#endif
	symrec *ptr = (symrec *) malloc(sizeof(symrec));
	ptr->name = (char *) malloc(strlen(sym_name) + 1);
	strcpy(ptr->name, sym_name);
	ptr->type = sym_type;
	ptr->value.var = 0;	/* set value to 0 even if fctn. */
	ptr->next = (struct symrec *)sym_table;
	sym_table = ptr;

/*	pdebug("PARSER: putsym() DONE"); */
	return ptr;
}

symrec* getsym (const char *sym_name) {
#ifdef PDEBUG
	printf("PARSER: getsym(): sym_name = %s\n", sym_name);
#endif
	symrec *ptr;
	for (ptr = sym_table; ptr != (symrec *) 0; ptr = (symrec *)ptr->next) {
		if (strcmp (ptr->name, sym_name) == 0) {
#ifdef PDEBUG
			printf("PARSER: symbol \'%s\' found\n", sym_name);
#endif
			return ptr;
		}
	}

#ifdef PDEBUG
	printf("PARSER: symbol \'%s\' not found\n", sym_name);
#endif
	return 0;
}

symrec* assign_variable(const char* symb_name, double value) {
#ifdef PDEBUG
	printf("PARSER: assign_variable(): symb_name = %s value=%g\n", symb_name, value);
#endif

	symrec* ptr = getsym(symb_name);
	if (!ptr) {
#ifdef PDEBUG
	printf("PARSER: calling putsym(): symb_name = %s\n", symb_name);
#endif

		ptr = putsym(symb_name, VAR);
	}

	ptr->value.var = value;

	return ptr;
};

static int getcharstr(void) {
/*	pdebug("PARSER: getcharstr()"); */

	if (string[pos] == '\0')
		return EOF;
	return (int)string[pos++];
}

static void ungetcstr(void) {
    if (pos > 0)
        pos--;
}

int init_parser() {
	init_table();

	return 0;
}

int yylex(void) {
/*	pdebug("PARSER: yylex()"); */
	int c;

	/* skip white space  */
	while ((c = getcharstr ()) == ' ' || c == '\t');

	/* finish if reached EOF */
	if (c == EOF) {
/*		pdebug("FINISHED"); */
		return 0;
	}

#ifdef PDEBUG
	printf("PARSER: reading character: %c\n", c);
#endif

	/* process numbers */
	if (isdigit(c)) {
/*		pdebug("PARSER: reading number (starts with digit)"); */
                ungetcstr();
                char *s = &string[pos];

		/* convert to double */
		char *remain;
#if defined(_WIN32) || defined(__APPLE__)
		double result = strtod(s, &remain);
#else
		/* use same locale for all languages: '.' as decimal point */
		locale_t locale = newlocale(LC_NUMERIC_MASK, "C", NULL);

		double result = strtod_l(s, &remain, locale);
		freelocale(locale);
#endif
#ifdef PDEBUG
		printf("PARSER: reading: %s", s);
		printf("PARSER: remain = %s", remain);
#endif
		/* check conversion */
		if(strlen(s) == strlen(remain))
			return 0;
#ifdef PDEBUG
		printf("PARSER: result = %g\n", result);
#endif
		yylval.dval = result;

                pos += strlen(s) - strlen(remain);

		return NUM;
	}

	if (isalpha (c) || c == '.') {
/*		pdebug("PARSER: reading identifier (starts with alpha)"); */
		static char *symbuf = 0;
		static int length = 0;
		int i=0;

		/* Initially make the buffer long enough for a 4-character symbol name */
		if (length == 0)
			length = 20, symbuf = (char *)malloc(length + 1);

		do {
			/* If buffer is full, make it bigger */
			if (i == length) {
				length *= 2;
				symbuf = (char *)realloc(symbuf, length + 1);
			}
			/* Add this character to the buffer */
			symbuf[i++] = c;
			/* Get another character */
			c = getcharstr();
		}
		while (c != EOF && (isalnum(c) || c == '_' || c == '.'));

		ungetcstr();
		symbuf[i] = '\0';

		symrec *s = getsym(symbuf);
		if(s == 0) {	/* symbol unknown */
#ifdef PDEBUG
			printf("PARSER: ERROR: symbol \"%s\" UNKNOWN\n", symbuf);
#endif
			return 0;
		}
		/* old behavior */
		/* if (s == 0)
			 s = putsym (symbuf, VAR);
		*/
		yylval.tptr = s;
		return s->type;
	}

	/* else: single operator */
/*	pdebug("PARSER: single operator"); */
	return c;
}
