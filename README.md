Installation
===========
` $ make `

Usage
=====
` $ ./ciscal -h `

Ciscal compiler.


    Usage: ciscal [options] ...
    Options:
        -f filename  	 	    Compiles filename. 
        -i 			 		    Output icode to icode.txt 
        -a [x86 or metasim] 	Output assembly to assembly.s. Default metasim.


For intermediate code and metasim final code

`$ ./ciscal -f test7.c -i -ametasim `

For intermediate and final code for AT&T x86 assembly

`$ ./ciscal -f test7.c -i -ax86 `
