/**
 * (C) Copyright 2010. All rights reserved. Sotiris Karavarsamis and Panagiotis Tigas.
 *  {cs061205, cs0526}@cs.uoi.gr
 * Computer Science Department
 * University of Ioannina
 * 
 * icode
 */

#ifndef IC_H
#define IC_H

#define die(x) { \
	fprintf(stderr, "error: %s\r\n", x); \
	exit(1); \
	} \

struct list {
	char name[35];
	struct list *next;
	int exitflag;
};

#define LIST struct list

/*
 * Attributes Struct
 */

struct attribute{
	char place[35];
	struct list *true;
	struct list *false;
};

/* 
 * Quads definitions
 */
#define QUAD struct quad*

struct quad {
	char op[35];
	char x[35], y[35], z[35];
	
	char label[35];
	QUAD next;
	QUAD prev;
};

#define ATTR struct attribute

/* true and false alias */

#define TRUE true
#define FALSE false

char * int2string(int);
int nextquad();
QUAD genquad(char*, char*, char*, char*);
void printquads();
char * newtemp(char *t);
struct list * makelist(char *x);
void printlist(struct list* l);
struct list * merge(struct list *a, struct list *b);
void backpatch(struct list* l, char* s);
struct list * exitlist(struct list *l);
void addtoquads(QUAD q, QUAD *quads, QUAD *tail);
QUAD newquad(char *op, char *x, char *y, char *z);
void fixquadlabels(int from, int to);

extern int qlabel;
extern int tempcounter;
extern QUAD quads;
extern QUAD tail;
extern QUAD gqp;
extern QUAD generated_ptr;

#endif
