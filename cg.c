#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "st.h"
#include "ic.h"
#include "cg.h"

/* current nesting level */
int ncur = 0;

char prog_name[255];

int is_const(char *s) {
	unsigned int i;
	
	for(i=0; i<strlen(s); i++) {
		if(!isdigit(s[i])) {
			return 0;
		}
	}
	
	return 1;
}

char * label( char *l ){
    char s[35];
    sprintf(s, "L%s", l);
    return strdup(s);
}

int generated_lines = 0;

extern int main_start_quad;

void print_machine_code(){
    CODE *c = mc_tail;
	FILE *out = fopen("assembly.s", "w");
	if (out == NULL) {
		fprintf(stderr, "Error: Can't open assembly. for writing.\n");
		exit(1);
	}
	
	/* HACK */
	fprintf(out, "L0:\tjmp L%d\n", main_start_quad);
	
    while(c != NULL){
		fprintf(out, "%s", c->cmd);
		c = c->prev;
	}
	
	fclose(out);
}

void add_machine_code( char *s ){
    CODE *tmp;
    
    tmp = (CODE*)malloc(sizeof(CODE));
    sprintf(tmp->cmd, "%s", s);
    
    if ( mc == NULL ){
        tmp->next = NULL;
        tmp->prev = NULL;
        mc = tmp;
        mc_tail = tmp;
    }else{
        mc->prev = tmp;
        tmp->next = mc;
        mc = tmp;
    }
}

void gnlvcode(char *name) {
	char s_tmp[255];
	int i;
	
	SYMBOL *p = lookup(name);
	
	sprintf(s_tmp, "\tmovi R[255], M[R[0]+4]\n");
	add_machine_code( s_tmp );
	
	for(i=p->level; i<ncur-1; i++) {
		sprintf(s_tmp, "\tmovi R[255], M[R[55]+4]\n");
		add_machine_code( s_tmp );
	}
	
	sprintf(s_tmp, "\tmovi R[254], %d\n", p->offset);
	add_machine_code( s_tmp );
	sprintf(s_tmp, "\taddi R[255], R[254], R[255]\n");
	add_machine_code( s_tmp );
}

void load( char *R, char *a ){
	char s_tmp[255];
	
    if ( !isalpha(*a) && *a != '$' ){
        sprintf(s_tmp, "\tmovi %s, %s\n", R, a);
        add_machine_code( s_tmp );
    }else{
        SYMBOL *p = lookup(a);
		
		if (p->level == 1 && p->type != TYPE_TMP) { /* global */
			sprintf(s_tmp, "\tmovi %s, M[%d]\n", R, 600+p->offset);
	        add_machine_code( s_tmp );
		} else {
			if (p->level == ncur) {
				
				if (p->type == TYPE_ARG && p->arg.type == INOUT) {
					sprintf(s_tmp, "\tmovi R[255], M[R[0]+%d]\n", p->offset);
			        add_machine_code( s_tmp );
					sprintf(s_tmp, "\tmovi %s, M[R[255]]\n", R);
			        add_machine_code( s_tmp );
				} else {
					sprintf(s_tmp, "\tmovi %s, M[R[0]+%d]\n", R, p->offset);
			        add_machine_code( s_tmp );
				}
				return;
			}else if (p->level < ncur) {
				gnlvcode(p->name);
				if (p->type == TYPE_ARG && p->arg.type == INOUT) {
					sprintf(s_tmp, "\tmovi R[255], M[R[255]]\n");
			        add_machine_code( s_tmp );
					sprintf(s_tmp, "\tmovi %s, M[R[255]]\n", R);
			        add_machine_code( s_tmp );
				} else {
					sprintf(s_tmp, "\tmovi %s, M[R[255]]\n", R);
			        add_machine_code( s_tmp );	
				}	
				return;
			}
		}
    }
}

void store( char *R, char *a ){
	char s_tmp[255];
	
	SYMBOL *p = lookup(a);
	
	if (p->level == 1 && p->type != TYPE_TMP) { /* global */
		sprintf(s_tmp, "\tmovi M[%d], %s\n", 600+p->offset, R);
        add_machine_code( s_tmp );
	} else {
		if (p->level == ncur) {
			if (p->type == TYPE_ARG && p->arg.type == INOUT) {
				sprintf(s_tmp, "\tmovi R[255], M[R[0]+%d]\n", p->offset);
		        add_machine_code( s_tmp );
				sprintf(s_tmp, "\tmovi M[R[255]], %s\n", R);
		        add_machine_code( s_tmp );
			} else {
				sprintf(s_tmp, "\tmovi M[R[0]+%d], %s\n", p->offset, R);
		        add_machine_code( s_tmp );
			}
			return;
		}else if (p->level < ncur || p->type == TYPE_TMP) {
			gnlvcode(p->name);
			if ( p->type == TYPE_TMP || (p->type == TYPE_ARG && p->arg.type == INOUT)) {
				sprintf(s_tmp, "\tmovi R[255], M[R[255]]\n");
		        add_machine_code( s_tmp );
				sprintf(s_tmp, "\tmovi M[R[255]], %s\n", R);
		        add_machine_code( s_tmp );
			} else {
				sprintf(s_tmp, "\tmovi M[R[255]], %s\n", R);
		        add_machine_code( s_tmp );	
			}	
			return;
		}		
	}
   
}

int arguments = 0;

void generate_final() {
	char s_tmp[255];
	
	while(gqp != NULL){
		QUAD q = gqp;
		
		sprintf(s_tmp, "%s:\n", label(q->label));
		add_machine_code( s_tmp );
		
		if (!strcmp(q->op,"begin_block")) {	
			SYMBOL *tmp = lookup(q->x);
			assert(tmp != NULL);
			
			if (tmp->func.type == PROG) {

			}
			
			ncur = tmp->level;
		} else if (!strcmp(q->op,"end_block")) {
			SYMBOL *tmp = lookup(q->x);
			assert(tmp != NULL);
			
			if (tmp->func.type != PROG) {									
				sprintf(s_tmp, "\tjmp M[R[0]]\n");
            	add_machine_code( s_tmp );
			}
		} else if ( strcmp( q->op, "jump") == 0){
            /* jmp */
            sprintf(s_tmp, "\tjmp %s\n", label(q->z) );
            add_machine_code( s_tmp );

        } else if ( strcmp( q->op, "print") == 0){
            /* print */
			load("R[1]", q->x);
			sprintf(s_tmp, "\touti R[1]\n");
			add_machine_code( s_tmp );   
        } else if ( strcmp( q->op, ":=") == 0){
            /* assign */
            load("R[1]", q->x);
            store("R[1]", q->z);
        } else if ( strcmp( q->op, "+") == 0){
            /* add */
            load("R[1]", q->x);
            load("R[2]", q->y);
            sprintf(s_tmp, "\taddi R[3], R[1], R[2]\n" );
            add_machine_code( s_tmp );
            store("R[3]", q->z);
        } else if ( strcmp( q->op, "-") == 0){
            /* sub */
            load("R[1]", q->x);
            load("R[2]", q->y);
            sprintf(s_tmp, "\tsubi R[3], R[1], R[2]\n" );
            add_machine_code( s_tmp );
            store("R[3]", q->z);
        } else if ( strcmp( q->op, "*") == 0){
            /* mul */
            load("R[1]", q->x);
            load("R[2]", q->y);
            sprintf(s_tmp, "\tmuli R[1], R[1], R[2]\n" );
            add_machine_code( s_tmp );
            store("R[1]", q->z);
        } else if ( strcmp( q->op, "/") == 0){
            /* div */
            load("R[1]", q->x);
            load("R[2]", q->y);
            sprintf(s_tmp, "\tdivi R[3], R[1], R[2]\n" );
            add_machine_code( s_tmp );
            store("R[3]", q->z);
        } else if ( \
            strcmp( q->op, "=") == 0 ||\
            strcmp( q->op, "<") == 0 ||\
            strcmp( q->op, ">") == 0 ||\
            strcmp( q->op, "<=") == 0 ||\
            strcmp( q->op, ">=") == 0 || \
            strcmp( q->op, "<>") == 0 \
            ){
            /* cmp */
            load("R[1]", q->x);
            load("R[2]", q->y);

            sprintf(s_tmp, "\tcmpi R[1],R[2]\n" );
            add_machine_code( s_tmp );

            if ( strcmp( q->op, "=") == 0 ){
                sprintf(s_tmp, "\tje %s\n", label(q->z) );
                add_machine_code( s_tmp );
            }

            if ( strcmp( q->op, "<") == 0 ){
                sprintf(s_tmp, "\tja %s\n", label(q->z) );
                add_machine_code( s_tmp );
            }

            if ( strcmp( q->op, ">") == 0 ){
                sprintf(s_tmp, "\tjb %s\n", label(q->z) );
                add_machine_code( s_tmp );
            }

            if ( strcmp( q->op, "<=") == 0 ){
                sprintf(s_tmp, "\tjae %s\n", label(q->z) );
                add_machine_code( s_tmp );
            }

            if ( strcmp( q->op, ">=") == 0 ){
                sprintf(s_tmp, "\tjbe %s\n", label(q->z) );
                add_machine_code( s_tmp );
            }

            if ( strcmp( q->op, "<>") == 0 ){
                sprintf(s_tmp, "\tjne %s\n", label(q->z) );
                add_machine_code( s_tmp );
            }

        } else if ( strcmp( q->op, "call") == 0){
			SYMBOL *p = lookup( q->z );
			
			arguments -= p->func.num_arg;

			if ( p->level == ncur ) {
				sprintf(s_tmp, "\tmovi R[255], M[R[0]+4]\n");
	            add_machine_code( s_tmp );
	
				sprintf(s_tmp, "\tmovi M[R[0]+%d], R[255]\n", cur_scope->framelength+4);
	            add_machine_code( s_tmp );
	
			} else if ( p->level > ncur ) {
				sprintf(s_tmp, "\tmovi M[R[0]+%d], R[0]\n", cur_scope->framelength+4);
	            add_machine_code( s_tmp );
			}

            sprintf(s_tmp, "\tmovi R[255], %d\n", cur_scope->framelength );
            add_machine_code( s_tmp );

			sprintf(s_tmp, "\taddi R[0], R[255], R[0]\n");
            add_machine_code( s_tmp );

			sprintf(s_tmp, "\tmovi R[255], $\n");
            add_machine_code( s_tmp );

			sprintf(s_tmp, "\tmovi R[254], 15\n");
            add_machine_code( s_tmp );

			sprintf(s_tmp, "\taddi R[255], R[255], R[254]\n");
            add_machine_code( s_tmp );

			sprintf(s_tmp, "\tmovi M[R[0]], R[255]\n");
            add_machine_code( s_tmp );

			sprintf(s_tmp, "\tjmp L%d\n", p->func.genquad);  
			add_machine_code( s_tmp );
			
			sprintf(s_tmp, "\tmovi R[255], %d\n", cur_scope->framelength );
            add_machine_code( s_tmp );

			sprintf(s_tmp, "\tsubi R[0], R[0], R[255]\n");
            add_machine_code( s_tmp );

        } else if ( strcmp( q->op, "par") == 0){
			/* par */

            SYMBOL *p = lookup( q->y );

            if ( !strcmp(q->x, "CV") ){ /* CV */
				int d = cur_scope->framelength + 12 + 4*arguments; /* framelength+12+4*i, i=incremental number of parameter */
				arguments++;
				
                load("R[255]", q->y);
				                
				sprintf(s_tmp, "\tmovi M[R[0]+%d], R[255]\n", d);
                add_machine_code( s_tmp );      
            } else if ( !strcmp(q->x, "RET") ){ /* RET */
                /* TODO */
				sprintf(s_tmp, "\tmovi R[255], R[0]\n");
	            add_machine_code( s_tmp );
	
				sprintf(s_tmp, "\tmovi R[254], %d\n", p->offset);
	            add_machine_code( s_tmp );
	
				sprintf(s_tmp, "\taddi R[255], R[254], R[255]\n");
	            add_machine_code( s_tmp );
	
				sprintf(s_tmp, "\tmovi M[R[0]+%d], R[255]\n", cur_scope->framelength+8);
	            add_machine_code( s_tmp );
            } else { /* REF */
				int d = cur_scope->framelength + 12 + 4*arguments; /* framelength+12+4*i, i=incremental number of parameter */
				arguments++;
				
				if ( ncur == p->level ) {
					if (p->type == TYPE_ARG && p->arg.type ==  INOUT ) {
						sprintf(s_tmp, "\tmovi R[255], R[0]\n");
			            add_machine_code( s_tmp );

						sprintf(s_tmp, "\tmovi R[254], %d\n", p->offset);
			            add_machine_code( s_tmp );

						sprintf(s_tmp, "\taddi R[255], R[254], R[255]\n");
			            add_machine_code( s_tmp );
			
						sprintf(s_tmp, "\tmovi R[1], M[R[255]]\n");
			            add_machine_code( s_tmp );

						sprintf(s_tmp, "\tmovi M[R[0]+%d], R[1]\n", d);
			            add_machine_code( s_tmp );
						
					} else {
						sprintf(s_tmp, "\tmovi R[255], R[0]\n");
			            add_machine_code( s_tmp );

						sprintf(s_tmp, "\tmovi R[254], %d\n", p->offset);
			            add_machine_code( s_tmp );

						sprintf(s_tmp, "\taddi R[255], R[254], R[255]\n");
						add_machine_code( s_tmp );
						
						sprintf(s_tmp, "\tmovi M[R[0]+%d], R[255]\n", d);
			            add_machine_code( s_tmp );
					}
					
				} else {
					gnlvcode(p->name);
					if (p->type == TYPE_ARG && p->arg.type ==  INOUT) {
						sprintf(s_tmp, "\tmovi R[1], M[R[255]]\n");
			            add_machine_code( s_tmp );
				
						sprintf(s_tmp, "\tmovi M[R[0]+%d], R[1]\n", d);
			            add_machine_code( s_tmp );
						
					} else {
						sprintf(s_tmp, "\tmovi M[R[0]+%d], R[255]\n", d);
			            add_machine_code( s_tmp );
					}
				}
			}
        }else if ( strcmp( q->op, "halt") == 0){
			sprintf(s_tmp, "\thalt\n");
		    add_machine_code( s_tmp );
		    return;
		}else if ( strcmp( q->op, "retv") == 0){
            if( strcmp(q->x,"_") != 0 ){
                load("R[1]", q->x);
            }else{
                load("R[1]", "0");
            }
            sprintf(s_tmp, "\tmovi R[255], M[R[0]+8]\n");
            add_machine_code( s_tmp );
            sprintf(s_tmp, "\tmovi M[R[255]], R[1]\n");
            add_machine_code( s_tmp );

			sprintf(s_tmp, "\tjmp M[R[0]]\n");
        	add_machine_code( s_tmp );
        }		
		gqp = gqp->prev;
	}	
}

