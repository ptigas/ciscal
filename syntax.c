/**
 * (C) Copyright 2010. All rights reserved. Sotiris Karavarsamis and Panagiotis Tigas.
 *  {cs061205, cs0526}@cs.uoi.gr
 * Computer Science Department
 * University of Ioannina
 * 
 * parser
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "ic.h"
#include "syntax.h"
#include "lexer.h"
#include "st.h"
#include "cg.h"
#include "cg_x86.h"

#define DEBUG_LEX 0
#define DEBUG_SYN 0
#define DEBUG_LEVEL 16
#define DEBUG_SYMBOL 0

#define TEMP_CHAR "$"

extern char *assembly;

int inside_while = 0;

#define TK2STR(TK) tk2str[TK-NF_COMMENT-1])

int main_start_quad;

void debug_lex() {
	do {
		lex();
	} while(token == COMMENT);

	if(token == ERROR) {
		fprintf(stderr,"lex error \n");
		exit(1);
	}

	if( DEBUG_LEX ) {
		printf("debug lex:%s ", tk2str[token-NF_COMMENT-1]);
		if (token == INTEGER) {
			printf("(%d)", id_value);

		} else if (token == ID) {
			printf("(%s)", id_string);
		}

		printf("\n");
	}
}

int ismain = 0;

void debug(int level, const char *s){
	int i;
	if(DEBUG_SYN) {
		if(level<DEBUG_LEVEL){
			if(level>0) printf("|");
			for(i=0; i<level; i++) {
				printf("-");
			}
			printf("%s", s);
			if (strcmp(s, "CONSTANT") == 0) {
				printf(" (%d)", id_value);

			} else if (strcmp(s, "ID") == 0) {
				printf(" (%s)", id_string);
			}
			printf("\n");
		}

	}
}

void syntax_error(const char *s){
	fprintf(stderr, "syntax error line %d: %s\n", line, s);
	exit(1);
}

void parse() {
	line = 1;
	memset(id_string, 0, 30);

	debug_lex();
	program(0);
}

char program_name[35];

void program(int level) {
	char *tmp = malloc(35*sizeof(char));

	if(token == TK_PROGRAM) {
		debug(level,"[PROGRAM]");

		debug_lex();

		id(&tmp, level + 1);
		//start_code_generation(t
		
		ismain = 1;

		block(tmp, level+1);

	} else {
		/* syntax error program */
		syntax_error("program expected");
	}
}

int is_program_block = 1;

void id(char **buf, int level) { 
	*buf = (char *)malloc(35*sizeof(char));
	if(*buf == 0) {
		die("EMEM");
	}

	if(token == ID) {

		strcpy(*buf, id_string);

		debug(level,"ID");
		if(is_program_block == 1) {
			strcpy(program_name, id_string);
		}

		debug_lex();

	} else {
		/* syntax error id expected */
		syntax_error("id expected");
	}
}

void subprograms(int level) {
	char *tmp = malloc(35*sizeof(char));
	FTYPE type;
	
	while(token == TK_PROCEDURE || token == TK_FUNCTION) {
		
		if (token == TK_PROCEDURE) {
			type = PROC;
		} else {
			type = FUNC;
		}
		
		debug(level,"SUBPROGRAMS");
		debug_lex();

		id(&tmp, level+1);
		new_function(tmp, type);
		push_scope(tmp);
		funcbody(tmp, level+1);
	}
}

void block(char *name, int level) {
	LIST *S = (LIST*)malloc(sizeof(LIST));
    SYMBOL *tmp = NULL;
	/* initial block call */
	if(is_program_block == 1) {
		is_program_block = 0;

		if(token == TK_LBRAC) {
			debug(level,"BLOCK");
			push_scope(name);
			new_function(name, PROG);
			debug_lex(level+1);
			declerations(level+1);
			subprograms(level+1);
			
         	tmp = lookup(name);
         	tmp->func.genquad = qlabel;
			main_start_quad = qlabel;         
			
			genquad("begin_block", program_name, "_", "_");
			sequence(S, level+1);

			if(token == TK_RBRAC) {
				
				
				pop_scope();
				debug_lex();
			} else {
				/* syntax error lpar expected */
				syntax_error("a '}' expected");
			}
		} else {
			/* syntax error rpar expected */
			syntax_error("'{' expected");
		}
		backpatch(S->next, int2string(nextquad()));
		
		genquad("halt", "_", "_", "_");
		genquad("end_block", program_name, "_", "_");
		
		
	} else {

		if(token == TK_LBRAC) {
			debug(level,"BLOCK");
			
			debug_lex(level+1);

			declerations(level+1);
			subprograms(level+1);
            
            tmp = lookup(name);
            tmp->func.genquad = qlabel;

			genquad("begin_block", name, "_", "_");

            fprintf(stderr, "quad quad %s %d\n", tmp->name, tmp->func.genquad);
			
            sequence(S, level+1);
			
			if(token == TK_RBRAC) {
				
				if (!strcmp(assembly, "x86")) {
					generate_final_x86( program_name);
				} else {
					generate_final( program_name);
				}
				
				pop_scope();
				debug_lex();
			} else {
				/* syntax error lpar expected */
				syntax_error("a '}' expected");
			}
			
		} else {
			/* syntax error rpar expected */
			syntax_error("'{' expected");
		}
		backpatch(S->next, int2string(nextquad()));
		genquad("end_block", name,"_", "_");
	}
}

void declerations(int level) {
	if(token == TK_DECLARE) {
		debug(level,"DECLERATIONS");
		debug_lex();

		varlist(level+1);
		if(token == TK_ENDDECLARE) {
			debug_lex();

		} else {
			/* syntax error enddeclare */
			syntax_error("'enddeclare' expected");
		}
	}
}

void varlist(int level) {
	if(token == ID) {
		debug(level,"VARLIST");
		new_variable(id_string, 0);
		debug_lex();

		while(token == TK_COMMA){
			debug_lex();

			if(token == ID) {
				new_variable(id_string, 0);
				debug_lex();

			} else {
				/* syntax error id expected */
				syntax_error("ID expected");
			}
		}

	}
}

void funcbody(char *name, int level) {
	debug(level,"FUNCBODY");
	formalpars(level+1);
	block(name, level+1);
}

void formalpars(int level) {
	if(token == TK_LPAR) {
		debug(level,"FORMALPARS");
		debug_lex();
		formalparlist(level+1);

		if(token == TK_RPAR) {
			debug_lex();

		} else {
			/* syntax error par open */
			syntax_error("parentheses is open");
		}
	}
}

void formalparlist(int level) {
	debug(level,"FORMALPARLIST");
	formalparitem(level+1);

	//debug_lex();
	while (token == TK_COMMA) {
		debug(level,"FORMALPARLIST L");
		debug_lex();

		formalparitem(level+1);
	}
}

void formalparitem(int level) {
	char *tmp = malloc(35*sizeof(char));
	SYMBOL *s;
	PARTYPE type;
	
	debug(level,"FORMALPARITEM");

	if (token == TK_IN || token == TK_INOUT) {
		if (token == TK_IN) {
			type = IN;
		} else {
			type = INOUT;
		}
		
		debug_lex();
		
		id(&tmp, level+1);
		
		s = lookup(tmp);
		if (s != NULL && s->type == TYPE_FUNC) {
			fprintf(stderr, "error line %d: there is already a function or a procedure called '%s'\n", line, tmp);
			exit(1);
		}
		
		add_argument(tmp, type);

	} else {
		/* syntax error expected in id inout id */
		syntax_error("in id or inout id expected");
	}
}

void sequence(LIST *L, int level) {
	LIST *S = (LIST*)malloc(sizeof(LIST));
	LIST *tmp = (LIST*)malloc(sizeof(LIST));
	debug(level,"SEQUENCE");
	statement(S, level+1);
	tmp = S->next;
	while(token == TK_SCOL) {
		debug(level,"SEQUENCE L");
		//backpatch(tmp, int2string(nextquad()));
		debug_lex();
		statement(S, level+1);
		tmp = merge(tmp, S->next);
	}
	L->next = tmp;
	
}

void bracket_seq(LIST *S, int level) {
	if(token == TK_LBRAC) {
		debug_lex();
		sequence(S, level+1);
		if(token == TK_RBRAC) {
			debug_lex();

		} else {
			/* syntax error lbrac expected */
			syntax_error("right bracket expected");
		}
	} 
}

void statement(LIST *S, int level) {
	LIST *list;
	ATTR *E = (ATTR*)malloc(sizeof(ATTR));
	debug(level,"STATEMENT");
	if(token == ID) {
		assignment_stat(S, level+1);
	} else if(token == TK_IF) {
		if_stat(S, level+1);
		backpatch(S, int2string(nextquad()));
	} else if(token == TK_DO) {
		while_stat(S, level+1);
		S->next = NULL;
	} else if(token == TK_EXIT) {
		exit_stat(level+1);
		list = makelist(int2string(nextquad()));
		list->exitflag = 1;
		genquad("jump", "_", "_", "_");
		S->next = merge(S->next, list);
	} else if(token == TK_RETURN) {
		return_stat(E, level+1);
		S->next = NULL;
	} else if(token == TK_PRINT) {
		print_stat(level+1);
		S->next = NULL;
	} else if(token == TK_CALL) {
		call_stat(level+1);
		S->next = NULL;
	} else {
		S->next = NULL;
	}
}

void assignment_stat(LIST *R, int level) {
	SYMBOL *tmp;
	ATTR *E = (ATTR*)malloc(sizeof(ATTR));
	char id[35];

	E = (ATTR*)malloc(sizeof(ATTR));

	debug(level,"ASSIGNMENT");
	if(token == ID) {		
		strcpy(id, id_string);
		
		tmp = lookup(id);
		if (tmp == NULL) {
			fprintf(stderr, "error line %d: \'%s\' no such variable.\n", line, id);
			exit(1);
		} else {
			if (tmp->type == TYPE_FUNC || tmp->type == TYPE_CONST) {
				fprintf(stderr, "error line %d: \'%s\' is not assignable.\n", line, id);
				exit(1);
			}
		}
		debug_lex();
		
		if(token == TK_COL) {
			debug_lex();
			if(token == TK_EQ) {
				debug_lex();
				expression(E, level+1);
				genquad(":=", E->place, "_", id);
				R->next = NULL;
			} else {
				syntax_error("expecting assignment operator between variable and expression!");
			}
		} else {
			syntax_error("expecting ':=' operator between variable and expression!");
		}

	} else {
		syntax_error("expecting ID word before assignment operator!");
	}
}

void brackets_seq(LIST *S, int level) {
	debug(level,"BRACKETS SEQ");
	if(token == TK_LBRAC) {
		debug_lex();
		sequence(S, level+1);

		if(token == TK_RBRAC) {
			debug_lex();

		} else {
			syntax_error("'}' expected");
		}

	} else {
		syntax_error("'{' expected");
	}
}

void brack_or_stat(LIST *S, int level) {
	debug(level,"BRACK OR STAT");
	if(token == TK_LBRAC) {
		brackets_seq(S, level+1);
	} else {
		statement(S, level+1);
	}
}

void if_stat(LIST *S, int level) {
	ATTR *B = (ATTR*)malloc(sizeof(ATTR));
	LIST *S1 = (LIST*)malloc(sizeof(LIST));
	LIST *TAIL = (LIST*)malloc(sizeof(LIST));
	LIST *list;
	int p1, p2;

	debug(level,"IF");
	if(token == TK_IF) {
		debug_lex();

		if(token == TK_LPAR) {
			debug_lex();

			condition(B, level+1);
			p1 = nextquad();

			if(token == TK_RPAR) {
				debug_lex();
				brack_or_stat(S1, level+1);
				list = makelist(int2string(nextquad()));
				genquad("jump", "_", "_", "_");
				p2 = nextquad();
				elsepart(TAIL, level+1);
				backpatch(B->true, int2string(p1));
				backpatch(B->false, int2string(p2));
				S->next = merge(S1->next, list);
				S->next = merge(S->next, TAIL->next);
			} else {
				syntax_error("if right parentheses expected");
			}

		} else {
			syntax_error("left parentheses expected");
		}

	} else {
		syntax_error("if expected");
	}
}

void elsepart(LIST * TAIL, int level) {
	LIST* S2 = (LIST*)malloc(sizeof(LIST));
	debug(level,"ELSE PART");

	if(token == TK_ELSE) {
		debug_lex();
		brack_or_stat(S2, level+1);
		TAIL->next = S2->next;
	} else {
		TAIL->next = NULL;
	}
}

void while_stat(LIST * S, int level) {
	ATTR *B = (ATTR*)malloc(sizeof(ATTR));
	LIST *S1 = (LIST*)malloc(sizeof(LIST));
	LIST *EXIT;
	int p1, p2;
	debug(level,"WHILE STATEMENT");

	if(token == TK_DO) {
		p1 = nextquad();
		debug_lex();
		
		inside_while++;
		brack_or_stat(S1, level+1);
		inside_while--;
		
		if(token == TK_WHILE) {
			debug_lex();
			if(token == TK_LPAR) {
				debug_lex();
				condition(B, level+1);
				backpatch(B->true, int2string(p1));
				p2 = nextquad();
				backpatch(B->false, int2string(nextquad()));
				EXIT = exitlist(S1);
				if (EXIT != NULL) {
					EXIT->exitflag = 0;
					backpatch(EXIT, int2string(p2));
				}
				if(token == TK_RPAR) {
					debug_lex();
					S->next = S1->next;
				} else {
					syntax_error("missing ')' constant after while conditional statement.");
				}
			} else {
				syntax_error("expecting '(' after while conditional statement.");
			}
		} else {
			syntax_error("expecting 'while' after loop conditional statement.");
		}
	} else {
		syntax_error("expecting 'do'");
	}
}

void exit_stat(int level) {
	debug(level,"EXIT");
	
	if(token == TK_EXIT) {
		if (inside_while == 0) {
			fprintf(stderr, "line %d: 'exit' statement out of while statement.\n", line);
			exit(1);
		}
		lex();
	} else {
		syntax_error("expecting 'exit' word.");
	}
}

void call_stat(int level) {
	SYMBOL *tmp;
	char name[31];
	debug(level,"CALL");
	
	if(token == TK_CALL) {
		debug_lex();
		if(token == ID) {
			strcpy(name, id_string);
			tmp = lookup(name);
			if (tmp == NULL || (tmp != NULL && tmp->type != TYPE_FUNC)) {
				fprintf(stderr, "error line %d: \'%s\' no such procedure or function.\n", line, name);
				exit(ERROR_SEM);
			} 
			
			if (tmp->func.type != PROC) {
				fprintf(stderr, "error line %d: \'%s\' is not a procedure .\n", line, tmp->name);
				exit(1);
			}
			
			debug_lex();
			actualpars(name, level+1);
			genquad("call", "_", "_", name);
		}
		
	} else {
		syntax_error("expecting 'call' word.");
	}
}

void return_stat(ATTR *E, int level) {
	debug(level,"RETURN");
	
	if(cur_scope->from == NULL || cur_scope->from->func.type != FUNC) {
		fprintf(stderr, "line %d: return statement out of function.\n", line);
		exit(1);
	} else {
		cur_scope->from->func.has_return = 1;
	}

	if(token == TK_RETURN) {
		debug_lex();

		if(token == TK_LPAR) {
			debug_lex();
			expression(E, level+1);
			genquad("retv", E->place, "_", "_");			
			if(token == TK_RPAR) {
				debug_lex();

			} else {
				syntax_error("missing ')' operator after return expression.");
			}


		} else {
			syntax_error("missing '(' operator before return statement.");
		}

	} else {
		syntax_error("missing 'return' word before return statement.");
	}
}

void print_stat(int level) {
	ATTR *E = (ATTR*)malloc(sizeof(E));
	debug(level,"PRINT");

	if(token == TK_PRINT) {
		debug_lex();

		if(token == TK_LPAR) {
			debug_lex();
			expression(E, level+1);
			genquad("print", E->place, "_", "_");
			if(token == TK_RPAR) {
				debug_lex();

			} else {
				syntax_error("missing right parenthesis ')' in a print statement expression.");
			}

		} else {
			syntax_error("missing left parenthesis '(' in a print statement expression.");
		}

	} else {
		syntax_error("missing 'print' identifier in print statement.");
	}
}

void condition(ATTR *B, int level) {
	int quad;
	ATTR *Q1 = (ATTR*)malloc(sizeof(ATTR));
	ATTR *Q2 = (ATTR*)malloc(sizeof(ATTR));

	debug(level,"CONDITION");
	boolterm(Q1, level+1);
	B->true = Q1->true;
	B->false = Q1->false;	

	while(token == TK_OR) {
		debug_lex();
		quad = nextquad();
		boolterm(Q2, level+1);
		backpatch(B->false, int2string(quad));
		B->true = merge(B->true, Q2->true);
		B->false = Q2->false;
	}
}

void boolterm(ATTR *Q, int level) {
	int quad;
	ATTR *R1 = (ATTR*)malloc(sizeof(ATTR));
	ATTR *R2 = (ATTR*)malloc(sizeof(ATTR));

	debug(level,"BOOLTERM");
	boolfactor(R1, level+1);
	Q->true = R1->true;
	Q->false = R1->false;

	while(token == TK_AND) {
		debug_lex();
		quad = nextquad();
		boolfactor(R2, level+1);
		backpatch(Q->true, int2string(quad));
		Q->false = merge(Q->false, R2->false);
		Q->true = R2->true;
	}
}

void boolfactor(ATTR *R, int level) {
	ATTR *E1 = (ATTR*)malloc(sizeof(ATTR));
	ATTR *E2 = (ATTR*)malloc(sizeof(ATTR));
	ATTR *B = (ATTR*)malloc(sizeof(ATTR));
	char *relop = (char*)malloc(3*sizeof(char));

	debug(level,"BOOLFACTOR");

	if(token == TK_NOT) {
		debug_lex();

		if(token == TK_LBBRAC) {
			debug_lex();
			condition(B, level+1);

			if(token == TK_RBBRAC) {
				debug_lex();
				R->false = B->true;
				R->true = B->false;
			} else {
				syntax_error("'] expected");
			}

		} else {
			syntax_error("'[' expected");
		}

	} else if(token == TK_LBBRAC) {
		debug_lex();
		condition(B, level+1);

		if(token == TK_RBBRAC) {
			debug_lex();
			R->true = B->true;
			R->false = B->false;
		} else {
			syntax_error("'] expected");
		}

	} else {
		expression(E1, level+1);
		//relational_oper(level+1);
		if(token >= TK_EQ && token <= TK_DIF) {
			sprintf(relop, "%s", id_string);
			debug_lex();
		} else {
			syntax_error("relational operator expected");
		}
		expression(E2, level+1);
		R->true = makelist(int2string(nextquad()));
		genquad(relop, E1->place, E2->place, "_");
		R->false = makelist(int2string(nextquad()));
		genquad("jump", "_", "_", "_");
	}
}

void relational_oper(int level) {
	debug(level,"REL OP");

	if(token >= TK_EQ && token <= TK_DIF) {
		debug_lex();
	} else {
		syntax_error("relational operator expected");
	}
}

void expression(ATTR *E, int level) {
	char *w, *op;
	ATTR *T1, *T2;
	T1 = (ATTR*)malloc(sizeof(ATTR));
	T2 = (ATTR*)malloc(sizeof(ATTR));

	debug(level,"EXPRESSION");

	if(token == ADD_OP) {		
		op = strdup(id_string);
		debug_lex();		
		term(T1, level+1);
		w = newtemp(TEMP_CHAR);
		genquad(op, "0", T1->place, w);
		strcpy(T1->place, w);
	} else {
		term(T1, level+1);
	}

	while(token == ADD_OP) {
		op = strdup(id_string);
		op[1] = '\0';

		debug_lex();
		term(T2, level+1);

		w = newtemp(TEMP_CHAR);
		genquad(op, T1->place, T2->place, w);
		strcpy(T1->place, w);
	}

	strcpy(E->place, T1->place);
}

void term(ATTR *T, int level) {
	char *w, op[35];
	ATTR *F1, *F2;
	F1 = (ATTR*)malloc(sizeof(ATTR));
	F2 = (ATTR*)malloc(sizeof(ATTR));

	debug(level,"TERM");

	factor(F1, level+1);

	while(token == MUL_OP) {
		strcpy(op, id_string);
		debug_lex();		
		factor(F2, level+1);
		w = newtemp(TEMP_CHAR);
		genquad(op, F1->place, F2->place, w);
		strcpy(F1->place, w);
	}
	strcpy(T->place, F1->place);
}

void factor(ATTR *F, int level) {
	char *tmp = malloc(35*sizeof(char));
	ATTR *E = (ATTR*)malloc(sizeof(ATTR));
	debug(level,"FACTOR");

	if(token == INTEGER) {
		if (id_value > 32767) {
			syntax_error("integer not in range [-32767, 32767]");
		}
		strcpy(F->place, int2string(id_value));
		debug_lex();
	} else if(token == TK_LPAR) {
		debug_lex();
		expression(E, level+1);

		if(token == TK_RPAR) {
			debug_lex();
		} else {
			syntax_error("expecting ')'");
		}
		strcpy(F->place, E->place);
	} else {
		id(&tmp, level+1);
		
		if(token == TK_LPAR) {
			SYMBOL *t = lookup(tmp);
			if (t != NULL) {
				if (t->type != TYPE_FUNC) {
					fprintf(stderr, "error line %d: \'%s\' is not a function .\n", line, tmp);
					exit(1);
				} else {
					if (t->func.type == PROC) {
						fprintf(stderr, "error line %d: \'%s\' is not a function .\n", line, tmp);
						exit(1);
					}
				}
			} else {
				fprintf(stderr, "error line %d: \'%s\' is not a function .\n", line, tmp);
				exit(1);
			}
			
			actualpars(tmp, level+1);
			char *w = newtemp(TEMP_CHAR);
			genquad("par", "RET", w, "_");
			genquad("call", "_", "_", tmp);
			strcpy(F->place, w);
		} else {
			SYMBOL *t = lookup(tmp);
			
			if (t == NULL) {
				fprintf(stderr, "error line %d: \'%s\' is not such variable.\n", line, tmp);
				exit(1);
			}
			
			if ( t->type == TYPE_FUNC ) {
				
				/*
				if ( t->func.num_arg > 0 ) {
					fprintf(stderr, "error line %d: wrong number of arguments.\n");
					exit(1);
				}
				*/
				
				if (t->func.type == PROC) {
					fprintf(stderr, "error line %d: \'%s\' is not a function .\n", line, tmp);
					exit(1);
				}
				
				char *w = newtemp(TEMP_CHAR);
				genquad("par", "RET", w, "_");
				genquad("call", "_", "_", tmp);
				strcpy(F->place, w);

			} else {
				strcpy(F->place, tmp);
			}
		}		
	}
	
}

/*
void idtail(int level) {
	debug(level,"IDTAIL");
	if(token == TK_LPAR) {
		actualpars(level+1);
	}
}
*/

void actualpars(char *name, int level) {
	SYMBOL *tmp;
	
	debug(level,"ACTUALPARS");
	if(token == TK_LPAR) {
		debug_lex();
		actualparlist(name, level+1);
		if(token == TK_RPAR) {
			debug_lex();
		} else {
			syntax_error("expecting ')'");
		}
	} else {
		tmp = lookup(name);
		if (tmp->func.num_arg > 0) {
			fprintf(stderr, "line %d: wrong number of arguments\n", line);
			exit(1);
		}
	}
}

void actualparlist(char *name, int level) {
	SYMBOL *tmp;
	int from;
	int itemcounter = 0;
	
	QUAD item = (QUAD)malloc(sizeof(struct quad));
	QUAD item_list = NULL;
	QUAD item_list_tail = NULL;
	QUAD q;
	
	tmp = lookup(name);	
	
	from = nextquad()-1;
	
	debug(level,"ACTUALPARLIST");
	actualparitem(name, ++itemcounter, &item, level+1);
	
	addtoquads(item, &item_list, &item_list_tail);

	while(token == TK_COMMA) {
		debug_lex();
		actualparitem(name, ++itemcounter, &item, level+1);
		q = item;
		
		addtoquads(item, &item_list, &item_list_tail);
	}
	
	/* append item_list quads to main quads */
	q = item_list_tail;
	while (q != NULL) {
		addtoquads(q, &quads, &tail);
		q = q->prev;
	}
	
	/* HACK fix quads labels */
	fixquadlabels(from, nextquad());
	
	if (itemcounter != tmp->func.num_arg) {
		fprintf(stderr, "line %d: wrong number of arguments\n", line);
		exit(1);
	}
}

void actualparitem(char *name, int itemcount, QUAD *item, int level) {
	ATTR *E = (ATTR*)malloc(sizeof(ATTR));
	SYMBOL *tmp, *args;
	int i;
	
	debug(level,"ACTUALPARITEM");
	
	tmp = lookup(name);
	args = tmp->func.first_arg;
	
	if (itemcount > tmp->func.num_arg) {
		fprintf(stderr, "line %d: wrong number of arguments\n", line);
		exit(1);
	}
	
	for(i=0; i<itemcount-1; i++) {
		args = args->arg.next_arg;
	}
	
	if(token == TK_IN) {
		if (args->arg.type != IN) {
			fprintf(stderr, "error line %d: passing argument %d by value\n", line, itemcount);
			exit(1);
		}
		debug_lex();
		expression(E, level+1);
		
		*item = newquad("par", "CV", E->place, "_");
		
	} else if (token == TK_INOUT) {
		if (args->arg.type != INOUT) {
			fprintf(stderr, "error line %d: passing argument %d by reference.\n", line, itemcount);
			exit(1);
		}
		
		debug_lex();
		if (token == ID) {
			SYMBOL *tmp = lookup(id_string);
			
			if (tmp == NULL) {
				fprintf(stderr, "error line %d: '%s' no such variable\n", line, id_string);
				exit(1);
			}
			
			*item = newquad("par", "REF", id_string, "_");
			debug_lex();
		} else {
			syntax_error("expecting id");
		}
	} else {
		syntax_error("expecting in or inout keyword");
	}
	
}

void optional_sign(int level) {
	debug(level,"OPTIONAL SIGN");
	if(token == ADD_OP) {
		debug_lex();
	}
}