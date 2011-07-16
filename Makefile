# (C) copyright 2010. All rights reserved to Panagiotis Tigas, Sotiris Karavarsamis.
# Makefile for the lexer implementation.

CC=gcc
CFLAGS=-g -c -Wall
LDFLAGS=-lm
SOURCES=main.c lexer.c syntax.c ic.c st.c cg.c cg_x86.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ciscal

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
