/**
 * (C) Copyright 2010. All rights reserved. Sotiris Karavarsamis and Panagiotis Tigas.
 *  {cs061205, cs0526}@cs.uoi.gr
 * Computer Science Department
 * University of Ioannina
 * 
 * symbol table
 */

#ifndef SYMBOL_H
#define SYMBOL_H

typedef enum { IN, INOUT } PARTYPE;
typedef enum { TYPE_FUNC, TYPE_VAR, TYPE_CONST, TYPE_TMP, TYPE_ARG } T;
typedef enum {FUNC, PROC, PROG} FTYPE;

typedef struct symbol {
	char *name;
	int offset;
	int level;
	
	struct function {
		FTYPE type;
        int size;
        int genquad;
        struct symbol *args;
		struct symbol *first_arg;
        int num_arg;
        int has_return;
    } func;

	struct argument {
        PARTYPE type;    /* passing type */
        struct symbol *next_arg;
        struct symbol *func;
        int offset;
		int num;
    } arg;

	struct variable {
        int offset;
    } var;
	
	T type;
	struct symbol *next;
} SYMBOL;

typedef struct scope {
	char *name;
	int level;
	SYMBOL *symbols;
	SYMBOL *from; /* function or procedure which pushed the scope */
	struct scope *next;
	struct scope *prev;
	int framelength;
} SCOPE;

void push_scope(char *name);
void pop_scope();
void install_symbol(SYMBOL *sym);
void new_variable(char *name, int istmp);
void new_function(char *name, FTYPE type);
void add_argument(char *name, PARTYPE par);
SYMBOL * lookup(char *name);
void print_symbol_table();

extern SCOPE *global_scope;
extern SCOPE *cur_scope;

#endif
