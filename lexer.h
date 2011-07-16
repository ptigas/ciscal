/**
 * (C) Copyright 2010. All rights reserved. Sotiris Karavarsamis and Panagiotis Tigas.
 *  {cs061205, cs0526}@cs.uoi.gr
 * Computer Science Department
 * University of Ioannina
 * 
 * lexer implementation code
 */


#ifndef LEXER_H
#define LEXER_H

#define IS_FINITE(S) ((S>=ID)&&(S<=ERROR))

#define die(x) { \
	fprintf(stderr, "error: %s\r\n", x); \
	exit(1); \
	} \

/* non-finite states of recognition automaton */
enum {
    NF_START = 0, /* initial state */
    NF_ID,        /* ID recognition state */
    NF_NUMBER,    /* numeric value recognition state */
    NF_LT,        /* lower-than operator state */
    NF_GT,        /* greater-than operator state */
    NF_COMMENT    /* comment block recognition state */
};

/* char type definition */
enum chartype { digit = 0, alpha, white, addop, mulop, lt, gt, eq, slash, star, lbrac, rbrac, lbbrac, rbbrac, lpar, rpar, comma, scol, col, eof, other};

/* define some tokens */
enum tokentype {
    ID = NF_COMMENT+1,
    INTEGER,
    CHAR,
    WHITE,
    ADD_OP, MUL_OP,
    TK_EQ,
    TK_LT, TK_GT,
    TK_LTEQ , TK_GTEQ, TK_DIF,
    TK_SLASH, TK_STAR,
    TK_LBRAC, TK_RBRAC,
    TK_LBBRAC, TK_RBBRAC,
    TK_LPAR, TK_RPAR,
    TK_COMMA, TK_SCOL, TK_COL,
    TK_AND, TK_DECLARE, TK_DO, TK_ELSE, TK_ENDDECLARE, TK_EXIT, TK_PROCEDURE, TK_FUNCTION, TK_PRINT, TK_IF, TK_IN, TK_INOUT, TK_NOT, TK_PROGRAM, TK_OR, TK_RETURN, TK_WHILE, TK_CALL,
    TK_EOF,
    COMMENT,
    ERROR
};

int isAlpha(char i);
int isNum(char i);
int isAlplhanum(char i);
int isWhite(char i);
int isOperator(char i);

extern FILE *in;
extern int line;

void lex();

extern int state[NF_COMMENT+1][other+1];
extern char tk2str[][30];

enum tokentype token;
char id_string[32];
int id_value;

#endif
