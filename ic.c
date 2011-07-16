/**
 * (C) Copyright 2010. All rights reserved. Sotiris Karavarsamis and Panagiotis Tigas.
 *  {cs061205, cs0526}@cs.uoi.gr
 * Computer Science Department
 * University of Ioannina
 * 
 * icode implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "st.h"
#include "ic.h"

#define DEBUG_IC 0

/* 
 * Quads definitions
 */

int qlabel = 1;

QUAD quads = NULL;
QUAD tail = NULL;

/* generated quads pointer */
QUAD gqp = NULL;

int tempcounter = 0;

/*
 *
 */

char * int2string( int i ){
	char *s;
	int l;
	
	l = l<2?2:l;
    s = (char*)malloc(((int)ceil(log10(l))+1)*sizeof(char));
	memset(s, 0, 32);
	if(s == NULL) {
		die("not enough memory");
	}

	sprintf(s, "%d", i);
	return s;
}

/*
 * Return the number of the next quad
 */

int nextquad(){
	return qlabel;
}

/*
 * Generates a quad
 */

QUAD newquad(char *op, char *x, char *y, char *z){
	QUAD q;
	
	q = (QUAD)malloc(sizeof(struct quad));
	
	/* uid */
	sprintf(q->label,"%d",qlabel++);
		
	sprintf(q->op,"%s",op);
	sprintf(q->x,"%s",x);
	sprintf(q->y,"%s",y);
	sprintf(q->z,"%s",z);
	
	return q;
}

void addtoquads(QUAD q, QUAD *quads, QUAD *tail) {
	
	if(*quads == NULL){
		*quads = q;
		q->prev = NULL;
		q->next = NULL;
		*tail = *quads;
	}else{
		(*quads)->prev = q;
		q->next = *quads;
		*quads = q;
	}
}

QUAD genquad(char *op, char *x, char *y, char *z){
	QUAD q;

	q = newquad(op, x, y, z);
		
	addtoquads(q, &quads, &tail);
	
	if ( gqp == NULL ) {
		gqp = quads;
	}
	
	if( DEBUG_IC ){
		printf("%s\t: %s %s %s %s\n", q->label, q->op, q->x, q->y, q->z);
	}

	return q;
}

void printquads(){
	QUAD q = tail;
	FILE *out = fopen("icode.txt", "w");
	if (out == NULL) {
		fprintf(stderr, "Error: Can't open icode.txt for writing.\n");
		exit(1);
	}
	
	while(q != NULL){
		fprintf(out, "%s\t%s %s %s %s", q->label, q->op, q->x, q->y, q->z);
		
        fprintf(out, "\n");
		if( strcmp( q->op, "enduni" ) == 0 ){
            fprintf(out, "\n");
		}
		q = q->prev;
	}
	
	fclose(out);
}


void fixquadlabels(int from, int to){
	QUAD q = tail;
	int i = from;

	int fix = 0;
	while(q != NULL){
		if (!strcmp(q->label, int2string(from))) {
			fix = 1;
		}
		
		if (!strcmp(q->label, int2string(to))) {
			fix = 0;
		}
		
		if (fix) {
			sprintf(q->label, "%s", int2string(i++));
		}
		
		q = q->prev;
	}
}

/*
 * Generates a new temp with t prefix
 */

char * newtemp(char *t){
	char s[30], *w;
	
	sprintf(s, "%s_%d", t, tempcounter++);
	
    w = strdup(s);
	new_variable(w, 1);
    
	return strdup(s);
}

/*
 * Makes the list and adds x
 */

struct list * makelist(char *x){
	struct list* tmp = (struct list*)malloc(sizeof(struct list));
	strcpy(tmp->name, x);
	tmp->next = NULL;
	tmp->exitflag = 0;
	return tmp;
}

void printlist(struct list* l){
	fprintf(stderr, "[ ");
	while(l != NULL){
		fprintf(stderr, "%s ",l->name);
		l = l->next;
	}
	fprintf(stderr, "]\n");
}

struct list * exitlist(struct list *l) {
	while(l != NULL){
		if(l->exitflag == 1) {
			return l;
		}
		l = l->next;
	}
	return NULL;
}
 
/*
 * Merges two lists
 */

struct list * merge(struct list *a, struct list *b){
	struct list *t, *p;
	if(a == NULL){
		return b;
	}		
	if(b == NULL){
		return a;
	}
	t = a;
    p = t;
	while(t->next != NULL){
		t = t->next;
	}   
	t->next = b;
	return p;
}

/*
 * BACKBITCH
 */

void backpatch(struct list* l, char* s){
	struct list* t;
	QUAD q;
	t = l;
	while(t != NULL){		
		q = quads;		
		while(q != NULL){
			if(t->exitflag == 0 && !strcmp(q->label, t->name) && !strcmp(q->z,"_")){
				strcpy(q->z, s);
			}
			q = q->next;
		}	
		t = t->next;		
	}
}
