program main {
	declare
		a, b, c
	enddeclare

	procedure test(inout a) {
		a := 3;
	}
	
	a := 2;
	print(a);
	
	call test(inout a);
	
	print(a);
}