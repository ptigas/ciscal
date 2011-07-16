#!/bin/bash

for f in tests/*.c;
do
	echo 'testing ' $f
	./ciscal -f $f -i -ax86 2> /dev/null;
	gcc assembly.s
	./a.out
done