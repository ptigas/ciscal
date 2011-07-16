program test {
	declare
		a, b, c
	enddeclare

	procedure test2(in a, in b, in d) {
		c := 4;
		print(a);
		print(b);
		print(d);
	}
	
	call test2(in 1, in 2, in 3);
	b := c;
	print(b);
}