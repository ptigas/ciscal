#ifndef CG86_H
#define CG86_H

void generate_final_x86();
void print_machine_code_x86();

void add_machine_code_x86( char *s );
void start_code_generation_x86( char *prog_name);
void end_code_generation_x86();

void load_x86( char *R, char *a);
void store_x86( char *R, char *a );
void gnlvcode_x86(char *name);

char * label_x86( char *l );

#endif
