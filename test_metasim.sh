#!/bin/bash

for f in tests/*.c;
do
	echo 'testing ' $f
	./ciscal -f $f -i -a 2> /dev/null;
	metasim assembly.s
done