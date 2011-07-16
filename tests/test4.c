program main {
	declare
		a, b, c
	enddeclare

	procedure lala(inout rr) {
		procedure test(inout a) {
			a := 3;
		}
		
		call test(inout rr);
	}

	a := 5;
	print(a);
	call lala(inout a);
	print(a);
	
}