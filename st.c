/**
 * (C) Copyright 2010. All rights reserved. Sotiris Karavarsamis and Panagiotis Tigas.
 *  {cs061205, cs0526}@cs.uoi.gr
 * Computer Science Department
 * University of Ioannina
 * 
 * symbol table implementation
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "st.h"
#include "lexer.h"

int level = 1;
int offset;
int var_offset;

extern int qlabel;

SCOPE *global_scope = NULL;
SCOPE *cur_scope = NULL;

void push_scope(char *name) {
	SYMBOL *f;
	SCOPE *scope = (SCOPE *)malloc(sizeof(SCOPE));
	
	if ( !scope ) {
		die("memory error");
	}
		
	scope->level = level++;
	scope->name = strdup(name);
	scope->next = NULL;
	scope->prev = NULL;
	scope->symbols = NULL;
	scope->framelength = 12;
	
	if (cur_scope != NULL) {
		f = lookup(name);
		scope->from = f;
	} else {
		scope->from = NULL;
	}
	
	if ( cur_scope == NULL ) {
		cur_scope = scope;
		global_scope = cur_scope;
	} else {
		scope->prev = cur_scope;
		cur_scope->next = scope;
		cur_scope = scope;
	}
}

void pop_scope() {
	SYMBOL *cur, *arg;
	level--;
	
	if (cur_scope->from) {
		if(cur_scope->from->func.has_return == 0 && cur_scope->from->func.type == FUNC) {
			fprintf(stderr, "line %d: function %s has no return stat.\n", line, cur_scope->from->name);
			exit(1);
		}
	} 
	
	if(cur_scope->prev != NULL) {
		cur_scope->prev->next = NULL;
		cur_scope = cur_scope->prev;
		cur = cur_scope->symbols;
		
		while(cur != NULL) {
			//SYMBOL *tmp = cur;
			
			/* drop symbol */
			if (cur->type == TYPE_FUNC) {
				/* drop arguments */
				arg = cur->func.first_arg;
				while(arg != NULL) {
					//SYMBOL *tmp = arg;
					arg = arg->arg.next_arg;
					
					//free(tmp);
					//tmp = NULL;
				}
				//free(cur->func.args);
				//cur->func.args = NULL;
			}
			cur = cur->next;
			
			//free(tmp->name);
			//free(tmp);
		}
	}
}

void print_symbol_table() {
	SCOPE *cur;
	SYMBOL *sym, *arg;
	fprintf(stderr, "-------------\n");
	cur = global_scope;
	while (cur != NULL) {
		fprintf(stderr, "%d: %s\n", cur->level, cur->name);
		sym = cur->symbols;
		while (sym != NULL) {
			fprintf(stderr, "\t%s", sym->name);
			if (sym->type == TYPE_FUNC) {
				arg = sym->func.first_arg;
				fprintf(stderr, "(");
				while(arg != NULL) {
					fprintf(stderr, "%s ", arg->name);
					arg = arg->arg.next_arg;
				}
				fprintf(stderr, ")");
				
			} else if (sym->type == TYPE_VAR) {
				fprintf(stderr, "[V] offset %d", sym->offset);
			} else if (sym->type == TYPE_ARG) {
				fprintf(stderr, "[A] offset %d", sym->offset);
			} else {
				fprintf(stderr, "[E] offset %d", sym->offset);
			}
			fprintf(stderr, "\n");
			sym = sym->next;
		}
		fprintf(stderr, "\n");
		cur = cur->next;		
	}
	fprintf(stderr, "-------------\n");
}

void install_symbol(SYMBOL *sym) {
	
	if (sym->type == TYPE_FUNC && sym->func.type == PROG) {
		sym->level = 1;
	} else {
		
		if ( sym->type == TYPE_FUNC ) {
			sym->level = cur_scope->level+1;
		} else {
			sym->level = cur_scope->level;
		}

	}
	
	if(cur_scope->symbols == NULL) {
		cur_scope->symbols = sym;
	} else {
		sym->next = cur_scope->symbols;
		cur_scope->symbols = sym;
	}
}

void new_variable(char *name, int istmp) {
	SYMBOL *t;
	SYMBOL *sym = (SYMBOL*)malloc(sizeof(SYMBOL));
	
	sym->offset = cur_scope->framelength;
	cur_scope->framelength += 4;
		
	/* TODO: na dw an uparxei to name */
	t = lookup(name);
	if(t != NULL) {
		if(t->level == cur_scope->level) {
			if (t->type == TYPE_ARG) {
				fprintf(stderr, "error line %d: redecleration of argument %s\n", line, name);
				exit(1);
			} else {
				fprintf(stderr, "error line %d: redecleration of %s in the same scope.\n", line, name);
				exit(1);
			}
		}
	}
	
	if (istmp == 1) {
		sym->type = TYPE_TMP;
	} else {
		sym->type = TYPE_VAR;
	}
	
	sym->name = strdup(name);
	sym->next = NULL;
	
	install_symbol(sym);
}

void new_function(char *name, FTYPE type) {
	SYMBOL *t;
	SYMBOL *sym = (SYMBOL*)malloc(sizeof(SYMBOL));
	
	/* TODO: na dw an uparxei to name */
	t = lookup(name);
	if(t != NULL) {
		fprintf(stderr, "error line %d: redecleration of %s\n", line, name);
		exit(1);
	}
	
	sym->type = TYPE_FUNC;
	sym->name = strdup(name);
	sym->func.type = type;
	sym->func.num_arg = 0;
	sym->func.has_return = 0;
	sym->func.args = NULL;
	sym->next = NULL;
	
	install_symbol(sym);
}

void add_argument(char *name, PARTYPE par) {
	//SYMBOL *t;
	SYMBOL *sym = (SYMBOL*)malloc(sizeof(SYMBOL));
	
	sym->offset = cur_scope->framelength;
	
	/* TODO: na dw an uparxei to name */
	
	sym->type = TYPE_ARG;
	sym->name = strdup(name);
	sym->arg.type = par;
	sym->arg.num = ++cur_scope->prev->symbols->func.num_arg;
	sym->arg.next_arg = NULL;
	sym->arg.func = cur_scope->prev->symbols;
	sym->level = cur_scope->level;
	
	cur_scope->framelength += 4;
	
	install_symbol(sym);
	
	if( cur_scope->prev->symbols->func.args == NULL ) {
		cur_scope->prev->symbols->func.first_arg = sym;
		cur_scope->prev->symbols->func.args = sym;
	} else {
		cur_scope->prev->symbols->func.args->arg.next_arg = sym;
		cur_scope->prev->symbols->func.args = sym;
	}
}

SYMBOL * lookup(char *name) {
	SCOPE *cur;
	SYMBOL *sym;

	cur = cur_scope;
	
	while (cur != NULL) {
		sym = cur->symbols;
		while (sym != NULL) {
			if(!strcmp(sym->name, name)) {
				return sym;
			}
			sym = sym->next;
		}
		cur = cur->prev;		
	}
	
	return NULL;
}
