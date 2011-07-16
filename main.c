/**
 * (C) Copyright 2010. All rights reserved. Sotiris Karavarsamis and Panagiotis Tigas.
 * Computer Science Department
 * University of Ioannina
 * 
 * icode generation main code
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "lexer.h"
#include "syntax.h"
#include "st.h"
#include "ic.h"
#include "cg.h"
#include "cg_x86.h"

void help() {
	char str[] = "Ciscal compiler.\n\
	              Usage: ciscal [options] ...\n\
	              Options:\n -f filename \t\t \
	              Compiles filename. \n -i \t\t\t \
	              Output icode to icode.txt \n \
	              -a [x86 or metasim] \t \
	              Output assembly to assembly.s. \
	              Default metasim.";
	fprintf(stdout, "%s\n", str);
}

char *assembly = NULL;

int main(int argc, char *argv[]) {
	int index;
	int final = 0;
	int icode = 0;
	char *filename = NULL;
	int c;
	
	opterr = 0;

	while ((c = getopt(argc, argv, "f:ia::h")) != -1) {
		switch (c) {
			case 'h':
				help();
				return 0;
			case 'f':
				filename = optarg;
				in = fopen(filename, "r");
				continue;
			case 'i':
				icode = 1;
				continue;
			case 'a':
				final = 1;
				assembly = optarg;
				continue;
			case '?':
				if (optopt == 'a') {
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
					fprintf (stderr, "Setting to default assembly [metasym].\n");
					assembly = strdup("metasim");
					final = 1;
					continue;
				}
				return 1;
			default:
				abort();
		}
	}
	
	/* drop the extra arguments */
	for (index = optind; index < argc; index++){
		printf ("Non-option argument %s\n", argv[index]);
		return 0;
	}
	
	/* start parsing */
    parse();

	/* print the icode quads */
	if (icode) {
		printquads();
	}
	
	/* print the final code */
	if (final) {
		if (!strcmp(assembly, "x86")) {
			generate_final_x86();
			print_machine_code_x86();
		} else {
			generate_final();
			print_machine_code();
		}
	}
    
    return 0;
}
