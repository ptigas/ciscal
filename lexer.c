/**
 * (C) Copyright 2010. All rights reserved. Sotiris Karavarsamis and Panagiotis Tigas.
 *  {cs061205, cs051126}@cs.uoi.gr
 * Computer Science Department
 * University of Ioannina
 * 
 * lexer implementation code
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

FILE *in = NULL;

/* define state machine matrix */
int state[NF_COMMENT+1][other+1] = {
    /* START */         {NF_NUMBER, NF_ID, WHITE, ADD_OP, MUL_OP, NF_LT, NF_GT, TK_EQ, TK_SLASH, TK_STAR, TK_LBRAC, TK_RBRAC, TK_LBBRAC, TK_RBBRAC, TK_LPAR, TK_RPAR, TK_COMMA, TK_SCOL, TK_COL, TK_EOF, ERROR},
    /* NF_ID */         {NF_ID, NF_ID, ID, ID, ID, ID, ID, ID, ID, ID, ID, ID, ID, ID, ID, ID, ID, ID, ID, ID, ERROR},
    /* NF_NUMBER */     {NF_NUMBER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, TK_LT},
    /* NF_LT */         {TK_LT, TK_LT, TK_LT, TK_LT, TK_LT, TK_LT, TK_DIF, TK_LTEQ, TK_LT, TK_LT, TK_LT, TK_LT, TK_LT, TK_LT, TK_LT, TK_LT, TK_LT, TK_LT, TK_LT, TK_LT, TK_LT},
    /* NF_GT */         {TK_GT, TK_GT, TK_GT, TK_GT, TK_GT, TK_GT, TK_GT, TK_GTEQ, TK_GT, TK_GT, TK_GT, TK_GT, TK_GT, TK_GT, TK_GT, TK_GT, TK_GT, TK_GT, TK_GT, TK_GT, TK_GT},
    /* NF_COMMENT */    {NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, NF_COMMENT, ERROR, NF_COMMENT}
    //                  {NF_NUMBER, NF_ID, WHITE, OP, NF_LT, NF_GT, EQ, TK_SLASH, TK_STAR, TK_LBRAC, TK_RBRAC, TK_LBBRAC, TK_RBBRAC, TK_LPAR, TK_RPAR, ERROR}
};

/* tk2str table */
char tk2str[][30] = {
	                 "ID",        "INTEGER",       "CHAR",      "WHITE",        "ADD_OP",		"MUL_OP",       "TK_EQ",    "TK_LT",      "TK_GT",
                     "TK_LTEQ",   "TK_GTEQ",       "TK_DIF",    "TK_SLASH",     "TK_STAR",      "TK_LBRAC", 	"TK_RBRAC", "TK_LBBRAC",
					 "TK_RBBRAC", "TK_LPAR",       "TK_RPAR",   "TK_COMMA",     "TK_SCOL",      "TK_COL",    	"TK_AND",	"TK_DECLARE", "TK_DO",
					 "TK_ELSE",   "TK_ENDDECLARE", "TK_EXIT",   "TK_PROCEDURE", "TK_FUNCTION",  "TK_PRINT", 	"TK_IF",	"TK_IN",
					 "TK_INOUT",  "TK_NOT",        "TK_PROGRAM","TK_OR",        "TK_RETURN",    "TK_WHILE",		"TK_CALL",	"TK_EOF",	"COMMENT",
					 "ERROR"
                    };

/* define reserved */
char reserved[][31] = {
    "and",
    "declare",
    "do",
    "else",
    "enddeclare",
    "exit",
    "procedure",
    "function",
    "print",
    "if",
    "in",
    "inout",
    "not",
    "program",
    "or",
    "return",
    "while",
	"call"
};

int isAlpha(char i){
    return ((i >= 'A')&&(i <= 'Z'))||((i >= 'a')&&(i <= 'z'))||(i == '_');
}

int isNum(char i){
	return (i >= '0')&&(i <= '9');
}

int isAlplhanum(char i){
	return isAlpha(i)||isNum(i);
}

int isWhite(char i){
	return (i == '\n')||(i == ' ')||(i == '\t');
}

int isOperator(char i){
	return (i == '+')||(i == '-')||(i == '*')||(i == '/');
}

int line = 0;

char get_char() {
	char c;
	c = fgetc(in);				/* read one byte from input buffered stream */
	if(c == '\n') {
        line++;
    }
	return c;
}

void put_char(char c) {
	ungetc(c, in);				/* if we did not identify a comment block we backtrack 1 byte on the buffer */
	if(c == '\n') {
		line--;
    }
}

void lex() {
    char c, cold; 			         /* current character vatiable */
    int S;				             /* automaton state discrete variable */
    int i;                           /* storage stack buffer index */
    enum chartype ic;                /* character type variable */
    int j;

	
    if (in == NULL) {                /* read bytes from standard input if no other buffer is specified */
        in = stdin;                  /* set the input buffer to the standard input buffer if no other buffer is specified */
    }
    
    i = 0;                           /* initialize cursor of automaton to 0 */
    S = NF_START;	                 /* set initial state of automaton */

    
    while (1) {                      /* automaton infinite loop; loop until we reach to some terminal state */
		cold = c;
		c = get_char();
        //printf("\t%c\n", c);
        if (isAlpha(c)) {			 /* if the character is an character of the english alphabet (whatever the loweress of the case) */
            ic = alpha;				 /* then update the character type of the current character to the alpha id */
			
        } else if (isNum(c)) {		 /* ; if the character is numeric, then */
            ic = digit;              /* update the character type to 'digit' id */
			
        } else if (isWhite(c)) {	 /* ; if the character is a while character, then */
            ic = white;				 /* update the character type to 'white' id */
			
        } else if (isOperator(c)) {  /* ; if the character is an operator character */
                                     /* then update the character type to 'op' id */
            if(c == '+' || c == '-') {
                ic = addop;
            } else {
                ic = mulop;
            }
			
        } else {
            if (c == '[') {			 /* ; if the character is a left square bracket */
                ic = lbbrac;		 /* then update the charactet type to 'lbbrac' id */
            
			} else if (c == ']') {   /* ~ */
                ic = rbbrac;
            
			} else if (c == '{') {   /* ~ */
                ic = lbrac;
            
			} else if (c == '}') {   /* ~ */
                ic = rbrac;
            
			} else if (c == '(') {   /* ~ */
                ic = lpar;
            
			} else if (c == ')') {   /* ~ */
                ic = rpar;
            
			} else if (c == '<') {   /* ~ */
                ic = lt;
            
			} else if (c == '>') {   /* ~ */
                ic = gt;
            
			} else if (c == '=') {   /* ~ */
                ic = eq;
            
			} else if (c == ',') {   /* ~ */
                ic = comma;
            
			} else if (c == ';') {   /* ~ */
                ic = scol;
            
			} else if (c == ':') {   /* ~ */
                ic = col;

			} else if (c == -1) {    /* ~ */
                ic = eof;
				
			} else {                 /* ~ */
                ic = other;
            }
        }
		
        if (S != NF_COMMENT) {      /* if the current state does not refer to a comment block, */
            if (c == '/') {         /* check if we are at the start of a comment block */
                c = get_char();
                if (c == '*') {     /* if we read a '*' byte we assume having identified a comment block */
                    S = NF_COMMENT; /* update automaton state to a comment state */
                } else {
                    put_char(c);  /* if we did not identify a comment block we backtrack 1 byte on the buffer */
					c = '/';
                }
            }
		
        } else {					/* check termination of a comment block */
            if (c == '*') {         /* if we read a '*' byte, we request one more byte from the input stream */
                c = get_char();      /* request a new byte from the input stream */
                if (c == '/') {     /* if the byte is the '/' character, */
                    S = COMMENT;    /* we update our state to the comment-block state */
                    token = S;      /* and return TK_COMMENT to the caller of lex() */
                    return;
                } else {            /* otherwise */
                    put_char(c);    /* we backtrack one byte in the input stream and reprobe for another token */
					c = '*';
                }
            }
        }
		
		S = state[S][ic];           /* probe next state of our finite automaton */
		
        
        if (S != NF_COMMENT) {      /* if we are not in a comment state */
            if (S != WHITE){        /* while we omit white character */
                if (i < 30){        /* and the buffer index does not exceed 30 units */
                    id_string[i++] = c;    /* push the currently suspected character to the array stack */
                }

            } else {
                i = 0;				/* ; otherwise, re-initialize the index of the stack buffer */
                S = NF_START;          /* and re-initialize the automaton state to the starting state */
                continue;           /* and re-probe */
            }

            if (IS_FINITE(S) && S != WHITE) { /* if the current state is finite and is not a white-space state */
                if (S == ID) {                /* if the state refers to some alphanumeric id */
                    put_char(c);            /* , and backtrack on the input buffer */
					c = cold;
					i--;
					id_string[i] = '\0';			/* NULL-terminate the character buffer */
					
                    for(j=0; j < 18; j++) {
                        if (strcmp(reserved[j], id_string) == 0) { /* check if the currently allocated string on */
                            S = TK_AND + j;                 /* stack matches to some known reserved word. */
                            token = S;
                            return;                         /* if that is the case, return the corresponding state to the reserved word */
                        }
                    }
                }
				
                if (S == INTEGER) {                         /* if the state refers to some integer */
                    put_char(c);							/* , backtrack one byte */
					c = cold;
					i--;
					id_string[i] = '\0';			/* NULL-terminate the character buffer */
                    id_value = atoi(id_string);						/* , and update the 'value' extern variable to the numeric value of the numeric string. */
					
                }
				
                if (S == TK_GT || S == TK_LT ) {			/* If we reach a GT state we also backtrack one byte on the input buffer */
                    put_char(c);							/* -- actual backtrack action -- */
					c = cold;
					i--;
                }

				id_string[i] = '\0';			/* NULL-terminate the character buffer */
                i = 0;							/* re-initialize the allocation buffer index to zero */
                token = S;

                return;  						/* At the end of the day, we also return the state ID of the token that we identified */
            }
        }
    }
    
    token = -1;
    return;
}
