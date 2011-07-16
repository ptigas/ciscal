#ifndef CG_H
#define CG_H

#define CODE struct code
struct code{
    char cmd[255];
    CODE *next;
    CODE *prev;
};

CODE *mc; /* machine code list */
CODE *mc_tail;

void generate_final();
void print_machine_code();

void add_machine_code( char *s );

void load( char *R, char *a);
void store( char *R, char *a );
void gnlvcode(char *name);

char * label( char *l );

void getAR( SYMBOL *s );
void loadAddr( char *R, char *a );

extern int ncur;
extern char prog_name[255];
extern int arguments;
extern int generated_lines;

#endif
