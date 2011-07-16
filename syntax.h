/**
 * (C) Copyright 2010. All rights reserved. Sotiris Karavarsamis and Panagiotis Tigas.
 *  {cs061205, cs0526}@cs.uoi.gr
 * Computer Science Department
 * University of Ioannina
 * 
 * syntax
 */

#ifndef SYNTAX_H
#define SYNTAX_H

#include "ic.h"
#include "st.h"

#define ATTR struct attribute

void id(char **buf, int level);
void program(int level);
void block(char *buf, int level);
void declerations(int level);
void varlist(int level);
void subprograms(int level);
void func(int level);
void funcbody(char *name, int level);
void formalpars(int level);
void formalparlist(int level);
void formalparitem(int level);
void sequence(LIST *,int level);
void bracket_seq(LIST *, int level);
void brack_or_stat(LIST *,int level);
void statement(LIST *,int level);
void assignment_stat(LIST *,int level);
void if_stat(LIST *,int level);
void while_stat(LIST *,int level);
void elsepart(LIST *,int level);
void exit_stat(int level);
void call_stat(int level);
void return_stat(ATTR *, int level);
void print_stat(int level);
void actualpars(char *name, int level);
void actualparlist(char *name, int level);
void actualparitem(char *name, int itemcounter, QUAD *item, int level);
void condition(ATTR *,int level);
void boolterm(ATTR *,int level);
void boolfactor(ATTR *,int level);
void expression(ATTR *E, int level);
void term(ATTR *T, int level);
void factor(ATTR *F, int level);
void idtail(int level);
void relational_oper(int level);
void add_oper(int level);
void mul_oper(int level);
void optional_sign(int level);

void parse();

extern int main_start_quad;

#endif
