#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "st.h"
#include "ic.h"
#include "cg.h"
#include "cg_x86.h"

char * label_x86( char *l ){
    char s[35];
    sprintf(s, ".L%s", l);
    return strdup(s);
}

int main_start_quad;

void print_machine_code_x86(){
    CODE *c = mc_tail;
	FILE *out = fopen("assembly.s", "w");
	if (out == NULL) {
		fprintf(stderr, "Error: Can't open assembly. for writing.\n");
		exit(1);
	}
	
	/* HACK */
	
	fprintf(out , "\
.text\n\
.global main\n\
\n");

    while(c != NULL){
		fprintf(out, "%s", c->cmd);
		c = c->prev;
	}
	
	fprintf(out, "\
print:\n\
\t.att_syntax\n\
\tpushl %%eax\n\
\tpushl $s\n\
\t.intel_syntax\n\
\tcall printf\n\
\t.att_syntax\n\
\taddl $8, %%esp\n\
\t.intel_syntax\n\
\tret\n\
\t.att_syntax\n\
.data\n\
\ts: .asciz \"%%d\\n\"\n\
\n\
");
	
	fclose(out);
}
void add_machine_code_x86( char *s ){
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

/* ebx gets the address */
void gnlvcode_x86(char *name) {
	char s_tmp[255];
	int i;
	
	SYMBOL *p = lookup(name);
		
	sprintf(s_tmp, "\tmovl -4(%%ebp), %%ebx\n");
	add_machine_code_x86( s_tmp );
	
	
	for(i=p->level; i<ncur-1; i++) {
		
		sprintf(s_tmp, "\tmovl -4(%%ebx), %%ebx\n");
		add_machine_code_x86( s_tmp );
	}
	
	sprintf(s_tmp, "\tsubl $%d, %%ebx\n", p->offset);
	add_machine_code_x86( s_tmp );
    
}

void load_x86( char *R, char *a ){
	char s_tmp[255];
	
    if ( !isalpha(*a) && *a != '$' ){
        sprintf(s_tmp, "\tmovl $%s, %%%s\n", a, R);
        add_machine_code_x86( s_tmp );
    }else{
        SYMBOL *p = lookup(a);
		if (p->level == ncur) {

			if (p->type == TYPE_ARG && p->arg.type == INOUT) {
				sprintf(s_tmp, "\tmovl -%d(%%ebp), %%edx\n", p->offset);
		        add_machine_code_x86( s_tmp );

				sprintf(s_tmp, "\tmovl (%%edx), %%%s\n",  R);
		        add_machine_code_x86( s_tmp );
			} else {
				sprintf(s_tmp, "\tmovl  -%d(%%ebp), %%%s\n", p->offset, R);
		        add_machine_code_x86( s_tmp );
			}
			return;
		}else if (p->level < ncur) {
			

			
			gnlvcode_x86(p->name);
			
			if (p->type == TYPE_ARG && p->arg.type == INOUT) {
				
				sprintf(s_tmp, "\tmovl (%%ebx), %%ebx\n");
		        add_machine_code_x86( s_tmp );
				sprintf(s_tmp, "\tmovl (%%ebx), %%%s\n", R);
		        add_machine_code_x86( s_tmp );
			} else {
				sprintf(s_tmp, "\tmovl (%%ebx), %%%s\n", R);
		        add_machine_code_x86( s_tmp );	
			}	
			return;
		}
	
    }
}

void store_x86( char *R, char *a ){
	char s_tmp[255];
	
	SYMBOL *p = lookup(a);
	
	if (p->level == ncur) {
		if (p->type == TYPE_ARG && p->arg.type == INOUT) {
			sprintf(s_tmp, "\tmovl -%d(%%ebp), %%edx\n", p->offset);
	        add_machine_code_x86( s_tmp );
			sprintf(s_tmp, "\tmovl %%%s, (%%edx)\n", R);
	        add_machine_code_x86( s_tmp );
		} else {
			sprintf(s_tmp, "\tmovl %%%s, -%d(%%ebp)\n", R, p->offset);
	        add_machine_code_x86( s_tmp );
		}
		return;
	}else if (p->level < ncur || p->type == TYPE_TMP) {
		gnlvcode_x86(p->name);

		if ( p->type == TYPE_TMP || (p->type == TYPE_ARG && p->arg.type == INOUT)) {
			sprintf(s_tmp, "\tmovl (%%ebx), %%ebx\n");
	        add_machine_code_x86( s_tmp );
			sprintf(s_tmp, "\tmovl (%%ebx), %%%s\n", R);
	        add_machine_code_x86( s_tmp );
		} else {
			sprintf(s_tmp, "\tmovl %%%s, (%%ebx)\n", R);
	        add_machine_code_x86( s_tmp );	
		}	
		return;
	}		

}

void generate_final_x86() {
	char s_tmp[255];
	
	while(gqp != NULL){
		QUAD q = gqp;
		
		sprintf(s_tmp, "%s:\n", label(q->label));
		add_machine_code_x86( s_tmp );
		
		if (!strcmp(q->op,"begin_block")) {
			
			SYMBOL *tmp = lookup(q->x);
			assert(tmp != NULL);
			
			if (tmp->func.type == PROG) {
				sprintf(prog_name, "%s", tmp->name);
				
				sprintf(s_tmp, "main:\n");
	            add_machine_code_x86( s_tmp );

				sprintf(s_tmp, "\tmovl %%esp, %%ebp\n");
	            add_machine_code_x86( s_tmp );
	
				sprintf(s_tmp, "\tsubl $%d, %%esp\n", cur_scope->framelength);
	            add_machine_code_x86( s_tmp );
				
			} else {
				sprintf(s_tmp, "\tmovl %%ebp, %%esp\n");
	            add_machine_code_x86( s_tmp );
			}
			
			ncur = tmp->level;
		} else if (!strcmp(q->op,"end_block")) {
			fprintf(stderr, "\n");
			
			SYMBOL *tmp = lookup(q->x);
			assert(tmp != NULL);
			
			if (tmp->func.type != PROG) {
				sprintf(s_tmp, "\tmovl %%ebp, %%esp\n");
	            add_machine_code_x86( s_tmp );
								
				sprintf(s_tmp, "\tret\n");
	        	add_machine_code_x86( s_tmp );
			}
			
		} else if ( strcmp( q->op, "jump") == 0){

            /* jmp */
            sprintf(s_tmp, "\tjmp %s\n", label(q->z) );
            add_machine_code_x86( s_tmp );

        } else if ( strcmp( q->op, "print") == 0){
            /* print */
			load_x86("eax", q->x);
			
			sprintf(s_tmp, "\tsubl $%d, %%esp\n", cur_scope->framelength);
            add_machine_code_x86( s_tmp );

			sprintf(s_tmp, "\tcall print\n");
			add_machine_code_x86( s_tmp );
			
			sprintf(s_tmp, "\taddl $%d, %%esp\n", cur_scope->framelength);
            add_machine_code_x86( s_tmp );
			
            
        } else if ( strcmp( q->op, ":=") == 0){
            /* assign */
            load_x86("eax", q->x);
            store_x86("eax", q->z);

        } else if ( strcmp( q->op, "+") == 0){

            /* add */
            load_x86("eax", q->x);
            load_x86("edx", q->y);
            sprintf(s_tmp, "\taddl %%edx, %%eax\n" );
            add_machine_code_x86( s_tmp );
            store_x86("eax", q->z);

        } else if ( strcmp( q->op, "-") == 0){

            /* sub */
            load_x86("eax", q->x);
            load_x86("edx", q->y);
            sprintf(s_tmp, "\tsubl %%edx, %%eax\n" );
            add_machine_code_x86( s_tmp );
            store_x86("eax", q->z);

        } else if ( strcmp( q->op, "*") == 0){

            /* mul */
            load_x86("eax", q->x);
            load_x86("ecx", q->y);
            sprintf(s_tmp, "\timul %%ecx\n" );
            add_machine_code_x86( s_tmp );
            store_x86("eax", q->z);

        } else if ( strcmp( q->op, "/") == 0){

            /* div */
            load_x86("eax", q->x);
            load_x86("ecx", q->y);
            sprintf(s_tmp, "\tcltd\n\tidiv %%ecx\n" );
            add_machine_code_x86( s_tmp );
            store_x86("eax", q->z);

        } else if ( \
            strcmp( q->op, "=") == 0 ||\
            strcmp( q->op, "<") == 0 ||\
            strcmp( q->op, ">") == 0 ||\
            strcmp( q->op, "<=") == 0 ||\
            strcmp( q->op, ">=") == 0 || \
            strcmp( q->op, "<>") == 0 \
            ){

            /* cmp */

           	load_x86("eax", q->x);
            load_x86("edx", q->y);

            sprintf(s_tmp, "\tcmp %%edx, %%eax\n" );
            add_machine_code_x86( s_tmp );

            if ( strcmp( q->op, "=") == 0 ){
                sprintf(s_tmp, "\tje %s\n", label(q->z) );
                add_machine_code_x86( s_tmp );
            }

            if ( strcmp( q->op, "<") == 0 ){
                sprintf(s_tmp, "\tjl %s\n", label(q->z) );
                add_machine_code_x86( s_tmp );
            }

            if ( strcmp( q->op, ">") == 0 ){
                sprintf(s_tmp, "\tjg %s\n", label(q->z) );
                add_machine_code_x86( s_tmp );
            }

            if ( strcmp( q->op, "<=") == 0 ){
                sprintf(s_tmp, "\tjle %s\n", label(q->z) );
                add_machine_code_x86( s_tmp );
            }

            if ( strcmp( q->op, ">=") == 0 ){
                sprintf(s_tmp, "\tjge %s\n", label(q->z) );
                add_machine_code_x86( s_tmp );
            }

            if ( strcmp( q->op, "<>") == 0 ){
                sprintf(s_tmp, "\tjne %s\n", label(q->z) );
                add_machine_code_x86( s_tmp );
            }

        } else if ( strcmp( q->op, "call") == 0){
			SYMBOL *p = lookup( q->z );
			
			arguments -= p->func.num_arg;
 
			if ( p->level == ncur ) {
				sprintf(s_tmp, "\tmovl -4(%%ebp), %%edx\n");
	            add_machine_code_x86( s_tmp );
	
				sprintf(s_tmp, "\tmovl %%edx, -%d(%%ebp)\n", cur_scope->framelength+4);
	            add_machine_code_x86( s_tmp );
	
			} else if ( p->level > ncur ) {
				sprintf(s_tmp, "\tmovl %%ebp, -%d(%%ebp)\n", cur_scope->framelength+4);
	            add_machine_code_x86( s_tmp );
			}

			sprintf(s_tmp, "\tsubl $%d, %%ebp\n", cur_scope->framelength);
            add_machine_code_x86( s_tmp );

			sprintf(s_tmp, "\tmovl %%ebp, %%esp\n");
            add_machine_code_x86( s_tmp );

			sprintf(s_tmp, "\tpopl %%eax\n");
            add_machine_code_x86( s_tmp );

			sprintf(s_tmp, "\tcall L%d\n", p->func.genquad);  
			add_machine_code_x86( s_tmp );
			
			sprintf(s_tmp, "\taddl $%d, %%ebp\n", cur_scope->framelength );
            add_machine_code_x86( s_tmp );

			sprintf(s_tmp, "\tmovl %%ebp, %%esp\n");
            add_machine_code_x86( s_tmp );

        } else if ( strcmp( q->op, "par") == 0){
			/* par */

            SYMBOL *p = lookup( q->y );
			
            if ( !strcmp(q->x, "CV") ){ /* CV */
				int d = cur_scope->framelength + 12 + 4*arguments; /* framelength+12+4*i, i=incremental number of parameter */
				arguments++;
				
                load_x86("edx", q->y);
				                
				sprintf(s_tmp, "\tmovl %%edx, -%d(%%ebp)\n\n", d);
				
				//fprintf(stderr, "A %s %d %d\n", p->name, cur_scope->framelength, arguments);
				//exit(-1);
                add_machine_code_x86( s_tmp );      
            } else if ( !strcmp(q->x, "RET") ){ /* RET */

				sprintf(s_tmp, "\tmovl %%ebp, %%edx\n");
	            add_machine_code_x86( s_tmp );
				
				sprintf(s_tmp, "\taddl $-%d, %%edx\n", p->offset);
	            add_machine_code_x86( s_tmp );
	
				sprintf(s_tmp, "\tmovl %%edx, -%d(%%ebp)\n", cur_scope->framelength+8);
	            add_machine_code_x86( s_tmp );

            } else { /* REF */
				int d = cur_scope->framelength + 12 + 4*arguments; /* framelength+12+4*i, i=incremental number of parameter */
				arguments++;
				
				if ( ncur == p->level ) {
					if (p->type == TYPE_ARG && p->arg.type ==  INOUT ) {
						sprintf(s_tmp, "\tmovl %%ebp, %%edx\n");
			            add_machine_code_x86( s_tmp );

						sprintf(s_tmp, "\taddl $-%d, %%edx\n", p->offset);
			            add_machine_code_x86( s_tmp );
			
						sprintf(s_tmp, "\tmovl (%%edx), %%eax\n");
			            add_machine_code_x86( s_tmp );

						sprintf(s_tmp, "\tmovl %%eax, -%d(%%ebp)\n", d);
			            add_machine_code_x86( s_tmp );
						
					} else {
						sprintf(s_tmp, "\tmovl %%ebp, %%edx\n");
			            add_machine_code_x86( s_tmp );

						sprintf(s_tmp, "\taddl $-%d, %%edx\n", p->offset);
						add_machine_code_x86( s_tmp );
						
						sprintf(s_tmp, "\tmovl %%edx, -%d(%%ebp)\n", d);
			            add_machine_code_x86( s_tmp );
					}
					
				} else {
					gnlvcode(p->name);
					if (p->type == TYPE_ARG && p->arg.type ==  INOUT) {
						sprintf(s_tmp, "\tmovl (%%ebx), %%eax\n");
			            add_machine_code_x86( s_tmp );
				
						sprintf(s_tmp, "\tmovl %%eax, -%d(%%ebp)\n", d);
			            add_machine_code_x86( s_tmp );
						
					} else {
						sprintf(s_tmp, "\tmovl %%ebx, -%d(%%ebp)\n", d);
			            add_machine_code_x86( s_tmp );
					}
				}	
			}

        }else if ( strcmp( q->op, "halt") == 0){

			/*	
			sprintf(s_tmp, "\taddl $%d, %%esp\n", cur_scope->framelength );
            add_machine_code_x86( s_tmp );
		
			sprintf(s_tmp, "\tleave\n");
			add_machine_code_x86( s_tmp );
			*/
			
			sprintf(s_tmp, "\tmovl %%ebp, %%esp\n");
            add_machine_code_x86( s_tmp );
		
			sprintf(s_tmp, "\tret\n");
			add_machine_code_x86( s_tmp );
			return;
	
		}else if ( strcmp( q->op, "retv") == 0){
            if( strcmp(q->x,"_") != 0 ){
                load_x86("eax", q->x);
            }else{
                load_x86("eax", "0");
            }

			sprintf(s_tmp, "\tmovl -8(%%ebp), %%edx\n");
            add_machine_code_x86( s_tmp );

            sprintf(s_tmp, "\tmovl %%eax, (%%edx)\n");
            add_machine_code_x86( s_tmp );

			sprintf(s_tmp, "\tmovl %%ebp, %%esp\n");
            add_machine_code_x86( s_tmp );
            
			sprintf(s_tmp, "\tret\n");
        	add_machine_code_x86( s_tmp );
        }		
		gqp = gqp->prev;
	}	
}